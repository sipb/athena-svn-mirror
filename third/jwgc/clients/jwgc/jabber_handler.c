#include "notice.h"
#include "exec.h"
#include "mux.h"
#include "main.h"

void process_presence(jabconn conn, jabpacket packet);
void process_iq_get(jabconn conn, jabpacket packet);
void process_iq_set(jabconn conn, jabpacket packet);
void process_iq_result(jabconn conn, jabpacket packet);
void process_iq_error(jabconn conn, jabpacket packet);

void 
jab_on_packet_handler(conn, packet)
	jabconn conn;
	jabpacket packet;
{
	dprintf(dExecution, "Jabber packet handler called...\n");
	if (conn->dumpfd != -1) {
		char *id = xode_get_attrib(packet->x, "id");
		if (conn->dumpid == NULL ||
			(id != NULL && !strcmp(conn->dumpid, id))) {
			char *tmpstr;

			dprintf(dExecution, "Dumping to client...\n");
			tmpstr = xode_to_str(packet->x);
			if (tmpstr) {
				write(conn->dumpfd, tmpstr, strlen(tmpstr) + 1);
			}
			return;
		}
	}

	jabpacket_reset(packet);
	dprintf(dExecution, "Packet: %s\n",
			packet->x->name ? packet->x->name : "n/a");
	switch (packet->type) {
		case JABPACKET_MESSAGE: {
			xode x;
			char *cbody, *body, *ns, *convbody;
			int body_sz;

			body = NULL;

			dprintf(dExecution, "Received MESSAGE\n");

#ifdef USE_GPGME
			x = xode_get_tag(packet->x, "x");
			if (x) {
				ns = xode_get_attrib(x, "xmlns");
				if (ns && !strcmp(ns, NS_ENCRYPTED)) {
					cbody = xode_get_data(x);
					if (cbody) {
						body = JDecrypt(cbody);
						if (body) {
							trim_message(body);
						}
					}
				}
			}
#endif /* USE_GPGME */
			if (!body) {
				x = xode_get_tag(packet->x, "body");
				if (x) {
					cbody = xode_get_data(x);
					if (cbody) {
						body = strdup(cbody);
						trim_message(body);
					}
				}
			}

			if (!unicode_to_str(body, &convbody)) {
				convbody = strdup("");
			}
			body_sz = strlen(convbody);

			decode_notice(packet);
			dprintf(dExecution, "Rendering message...\n");
			exec_process_packet(program, packet, convbody, body_sz);
			free(body);
		} break;

		case JABPACKET_PRESENCE: {
			dprintf(dExecution, "Received PRESENCE\n");
			dprintf(dExecution, "Contact status change\n");
			process_presence(conn, packet);
		} break;

		case JABPACKET_IQ: {
			dprintf(dExecution, "IQ received: %d\n",
					packet->subtype);
			switch (packet->subtype) {
				case JABPACKET__GET: {
					dprintf(dExecution,
						"Process IQ Get...\n");
					process_iq_get(conn, packet);
				} break;

				case JABPACKET__SET: {
					dprintf(dExecution,
						"Process IQ Set...\n");
					process_iq_set(conn, packet);
				} break;

				case JABPACKET__RESULT: {
					dprintf(dExecution,
						"Process IQ Result...\n");
					process_iq_result(conn, packet);
				} break;

				case JABPACKET__ERROR: {
					dprintf(dExecution,
						"Process IQ Error...\n");
					process_iq_error(conn, packet);
				} break;

				default: {
					dprintf(dExecution,
						"Unknown subtype.\n");
				} break;
			}
		} break;

		default: {
			dprintf(dExecution,
			"unrecognized packet: %i recieved\n", packet->type);
		} break;
	}
}

void 
jab_on_state_handler(conn, state)
	jabconn conn;
	int state;
{
	static int previous_state = JABCONN_STATE_OFF;

	dprintf(dExecution,
	"Entering: new state: %i previous_state: %i\n", state, previous_state);
	switch (state) {
		case JABCONN_STATE_OFF: {
			if (previous_state != JABCONN_STATE_OFF) {
				dprintf(dExecution,
				"The Jabber server has disconnected you: %i\n",
				previous_state);
				mux_delete_input_source(jab_getfd(jab_c));
				jab_reauth = 1;
			}
		} break;

		case JABCONN_STATE_CONNECTED: {
			dprintf(dExecution, "JABCONN_STATE_CONNECTED\n");
		} break;

		case JABCONN_STATE_AUTH: {
			dprintf(dExecution, "JABCONN_STATE_AUTH\n");
		} break;

		case JABCONN_STATE_ON: {
			xode x, out;

			dprintf(dExecution, "JABCONN_STATE_ON\n");

			dprintf(dExecution, "requesting agent list\n");
			x = jabutil_iqnew(JABPACKET__GET, NS_AGENTS);
			jab_send(conn, x);
			xode_free(x);

			dprintf(dExecution, "requesting roster\n");
			x = jabutil_iqnew(JABPACKET__GET, NS_ROSTER);
			jab_send(conn, x);
			xode_free(x);

			dprintf(dExecution, "requesting agent info\n");
			x = jabutil_iqnew(JABPACKET__GET, NS_AGENT);
			jab_send(conn, x);
			xode_free(x);

			if (strcmp((char *)jVars_get(jVarPresence),
							"available")) {
				xode y;

				out = jabutil_presnew(JABPACKET__AVAILABLE,
					NULL,
					(char *)jVars_get(jVarPresence),
					*(int *)jVars_get(jVarPriority));
				x = xode_insert_tag(out, "show");
				y = xode_insert_cdata(x,
				(char *)jVars_get(jVarPresence),
				strlen((char *)jVars_get(jVarPresence)));
			}
			else {
				out = jabutil_presnew(JABPACKET__AVAILABLE,
					NULL,
					NULL,
					*(int *)jVars_get(jVarPriority));
			}
			jab_send(conn, out);
			xode_free(out);
		} break;

		default: {
			dprintf(dExecution, "UNKNOWN state: %i\n", state);
		} break;
	}
	previous_state = state;
}

void
process_presence(conn, packet)
	jabconn conn;
	jabpacket packet;
{
	char *from = packet->from->full;
	if (!from) {
		dprintf(dExecution, "Presence processing with no from.\n");
		return;
	}

	switch (packet->subtype) {
		case JABPACKET__SUBSCRIBE: {
			xode x;

			x = jabutil_presnew(JABPACKET__SUBSCRIBED,
				from, NULL, -1);
			jab_send(conn, x);
			decode_notice(packet);
			dprintf(dExecution,
				"Rendering subscribe message...\n");
			exec_process_packet(program, packet, "", 0);
			xode_free(x);
		} break;

		case JABPACKET__UNSUBSCRIBE: {
			xode x;

			x = jabutil_presnew(JABPACKET__UNSUBSCRIBED,
				from, NULL, -1);
			jab_send(conn, x);
			decode_notice(packet);
			dprintf(dExecution,
				"Rendering unsubscribe message...\n");
			exec_process_packet(program, packet, "", 0);
			xode_free(x);
		} break;

		case JABPACKET__SUBSCRIBED: {
			dprintf(dExecution,
				"Rendering subscribed message...\n");
			exec_process_packet(program, packet, "", 0);
		} break;

		case JABPACKET__UNSUBSCRIBED: {
			dprintf(dExecution,
				"Rendering unsubscribed message...\n");
			exec_process_packet(program, packet, "", 0);
		} break;

		case JABPACKET__INVISIBLE:
		case JABPACKET__UNAVAILABLE:
		case JABPACKET__AVAILABLE: {
			xode x;
			char *jid, *pos, *ns;
			int ret;

			ret = contact_status_change(packet);
			if (ret) {
				if (time(0) > (jab_connect_time + 30)) {
					dprintf(dExecution, "Decoding notice...\n");
					decode_notice(packet);
					dprintf(dExecution, "Rendering presence message\n");
					exec_process_packet(
						program,
						packet,
						"",
						0);
				}
			}

			jid = xode_get_attrib(packet->x, "from");
			if (!jid) {
				break;
			}

			pos = (char *) strchr(jid, '/');
			if (pos) {
				*pos = '\0';
			}

			x = xode_get_tag(packet->x, "x");
			if (!x) {
				break;
			}

			ns = xode_get_attrib(x, "xmlns");
			if (!ns) {
				break;
			}


			dprintf(dExecution, "presence ns: %s\n", ns);
#ifdef USE_GPGME
			if (!strcmp(ns, NS_SIGNED)) {
				char *signature, *status;

				dprintf(dExecution,
					"Processing gpg signature...\n");

				signature = xode_get_data(x);
				status = xode_get_tagdata(packet->x, "status");
				if (signature && status) {
					dprintf(dExecution,
						"Signature[%d] is:\n%s\n",
						jid, signature);
					JUpdateKeyList(jid, status, signature);
				}
			}
#endif /* USE_GPGME */
		} break;

		case JABPACKET__ERROR: {
			char *errormsg;

			errormsg = xode_get_tagdata(packet->x, "error");
			dprintf(dExecution,
				"Error from %s: %s\n",
				from ? from : "n/a",
				errormsg ? errormsg : "n/a");
		} break;

		default: {
			dprintf(dExecution,
				"Unknown or ignored subtype from %s.\n",
				from ? from : "n/a");
		} break;
	}
}

void
process_iq_result(conn, packet)
	jabconn conn;
	jabpacket packet;
{
	char *id, *ns;
	xode x, out;

	id = xode_get_attrib(packet->x, "id");
	if (!id) {
		dprintf(dExecution, "No ID!\n");
	}

	x = xode_get_tag(packet->x, "query");
	if (!x) {
		dprintf(dExecution, "No query\n");
		return;
	}

	ns = xode_get_attrib(x, "xmlns");
	if (!ns) {
		dprintf(dExecution, "No NS!\n");
		return;
	}

	dprintf(dExecution, "ns: %s\n", ns);
	if (strcmp(ns, NS_ROSTER) == 0) {
		xode y;

		y = xode_get_tag(x, "item");
		while (y) {
			char *jid, *sub, *name, *group;
			char *resource = NULL;
			char *pos;
			xode z;

			jid = xode_get_attrib(y, "jid");
			if (!jid)
				break;
			sub = xode_get_attrib(y, "subscription");
			name = xode_get_attrib(y, "name");
			z = xode_get_tag(y, "group");
			group = xode_get_data(z);
        
			pos = (char *) strchr(jid, '/');
			if (pos) {
				*pos = '\0';
				resource = pos + 1;
			}

			dprintf(dExecution, "Buddy: %s/%s(%s) %s %s\n",
				jid ? jid : "n/a",
				resource ? resource : "n/a",
				group ? group : "n/a",
				sub ? sub : "n/a",
				name ? name : "n/a");
			update_contact_status(jid, NULL, resource);
			if (name) {
				update_nickname(jid, name);
			}
			if (group) {
				update_group(jid, group);
			}
			y = xode_get_nextsibling(y);
		}
	}
	else if (strcmp(ns, NS_AGENTS) == 0) {
		xode y;

		y = xode_get_tag(x, "agent");
		while (y) {
			char *jid, *name, *service;
			int flags = AGENT_NONE;

			jid = xode_get_attrib(y, "jid");
			if (!jid)
				break;
			name = xode_get_tagdata(y, "name");
			service = xode_get_tagdata(y, "service");
			if (xode_get_tag(y, "transport")) {
				flags |= AGENT_TRANSPORT;
			}
			if (xode_get_tag(y, "register")) {
				flags |= AGENT_REGISTER;
			}
			if (xode_get_tag(y, "groupchat")) {
				flags |= AGENT_GROUPCHAT;
			}
			if (xode_get_tag(y, "search")) {
				flags |= AGENT_SEARCH;
			}
			insert_into_agent_list(jid, name, service, flags);
			dprintf(dExecution, "Agent[%s]: %s [%s]\n",
				name ? name : "n/a",
				jid ? jid : "n/a",
				service ? service : "n/a");
			y = xode_get_nextsibling(y);
		}
	}
	else if (strcmp(ns, NS_AGENT) == 0) {
		char *name, *url, *reg;

		name = xode_get_tagdata(x, "name");
		url = xode_get_tagdata(x, "url");
		reg = xode_get_tagdata(x, "register");
		dprintf(dExecution, "agent: %s - %s\n",
			name ? name : "n/a",
			url ? url : "n/a");
	}
	else if (strcmp(ns, NS_VERSION) == 0) {
		char *name, *ver, *os;

		name = xode_get_tagdata(x, "name");
		ver = xode_get_tagdata(x, "version");
		os = xode_get_tagdata(x, "os");
		dprintf(dExecution, "version: %s - %s - %s\n",
			name ? name : "n/a",
			ver ? ver : "n/a",
			os ? os : "n/a");
	}
	else if (strcmp(ns, NS_AUTH) == 0) {
		char *seq, *tok;
		xode password, digest;

		password = xode_get_tag(x, "password");
		if (password)
			conn->auth_password = 1;
		else
			conn->auth_password = 0;

		digest = xode_get_tag(x, "digest");
		if (digest)
			conn->auth_digest = 1;
		else
			conn->auth_digest = 0;

		seq = xode_get_tagdata(x, "sequence");
		if (seq)
			conn->sequence = atoi(seq);
		else
			conn->sequence = 0;

		tok = xode_get_tagdata(x, "token");
		if (tok)
			conn->token = tok;
		else
			conn->token = NULL;

		if (seq && tok)
			conn->auth_0k = 1;
		else
			conn->auth_0k = 0;

		/* broken currently */
		/* conn->auth_digest = 0; */

		dprintf(dExecution,
			"auth support:\npassword=%s\ndigest=%s\n0k=%s %s %s\n",
			password ? "supported" : "not supported",
			digest ? "supported" : "not supported",
			(seq && tok) ? "supported" : "not supported",
			(seq && tok) ? seq : "",
			(seq && tok) ? tok : "");
	}

	return;
}

void
process_iq_set(conn, packet)
	jabconn conn;
	jabpacket packet;
{
	xode x;
	char *ns;
	
	x = xode_get_tag(packet->x, "query");
	ns = xode_get_attrib(x, "xmlns");
	if (strcmp(ns, NS_ROSTER) == 0) {
		xode y;
		char *jid, *sub, *name, *ask;

		y = xode_get_tag(x, "item");
		jid = xode_get_attrib(y, "jid");
		sub = xode_get_attrib(y, "subscription");
		name = xode_get_attrib(y, "name");
		ask = xode_get_attrib(y, "ask");
		dprintf(dExecution, "iq set request: %s %s %s %s\n",
			jid ? jid : "n/a",
			sub ? sub : "n/a",
			name ? name : "n/a",
			ask ? ask : "n/a");

		if (sub) {
			if (!strcmp(sub, "remove") || !strcmp(sub, "none")) {
				if (!jid) {
					dprintf(dExecution,
						"No jid specified!\n");
					return;
				}
				remove_from_contact_list(jid);
				dprintf(dExecution, "Contact %s removed\n", jid);
				return;
			}
		}
	}
}

void
process_iq_get(conn, packet)
	jabconn conn;
	jabpacket packet;
{
}

void
process_iq_error(conn, packet)
	jabconn conn;
	jabpacket packet;
{
	xode x;
	char *code, *desc;

	x = xode_get_tag(packet->x, "error");
	code = xode_get_attrib(x, "code");
	desc = xode_get_tagdata(packet->x, "error");
	dprintf(dExecution, "Received error: %s\n",
			desc ? desc : "n/a");
	fprintf(stderr, "Error %s: %s\n",
			code ? code : "n/a",
			desc ? desc : "n/a");
	switch (atoi(code)) {
		case 302:	/* Redirect */
			break;
		case 400:	/* Bad request */
			/* mux_end_loop_p = 1; */
			break;
		case 401:	/* Unauthorized */
			/* mux_end_loop_p = 1; */
			break;
		case 402:	/* Payment Required */
			/* mux_end_loop_p = 1; */
			break;
		case 403:	/* Forbidden */
			/* mux_end_loop_p = 1; */
			break;
		case 404:	/* Not Found */
			/* mux_end_loop_p = 1; */
			break;
		case 405:	/* Not Allowed */
			/* mux_end_loop_p = 1; */
			break;
		case 406:	/* Not Acceptable */
			/* mux_end_loop_p = 1; */
			break;
		case 407:	/* Registration Required */
			/* mux_end_loop_p = 1; */
			break;
		case 408:	/* Request Timeout */
			/* mux_end_loop_p = 1; */
			break;
		case 409:	/* Conflict */
			/* mux_end_loop_p = 1; */
			break;
		case 500:	/* Internal Server Error */
			break;
		case 501:	/* Not Implemented */
			break;
		case 502:	/* Remote Server Error */
			break;
		case 503:	/* Service Unavailable */
			break;
		case 504:	/* Remote Server Timeout */
			break;
		default:
			break;
	}
}
