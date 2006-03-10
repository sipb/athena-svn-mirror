#include "notice.h"
#include "exec.h"
#include "mux.h"
#include "main.h"

void 
jwg_on_event_handler(jwgconn conn, jwgpacket packet)
{
	dprintf(dExecution, "JWGC event handler called...\n");

	jwgpacket_reset(packet);
	dprintf(dExecution, "Received event: %d.\n", packet->type);
	switch (packet->type) {
		case JWGPACKET_PING: {
			char *match, *target, *message, *dontmatch, *type;
			char retstr[80];
			xode out;

			target = xode_get_attrib(packet->x, "to");
			dontmatch = xode_get_attrib(packet->x, "dontmatch");
			type = xode_get_attrib(packet->x, "type");
			if (!type) {
				type = strdup("chat");
			}

			dprintf(dExecution, "Received ping\n");

			if (!target) {
				dprintf(dExecution, "No target specified.\n");
				jwg_serverror(conn, "No target specified.");
				break;
			}

			dprintf(dExecution, "Target %s\n", target);
			if (dontmatch && !strcmp(dontmatch, "yes")) {
				match = strdup(target);
			}
			else if (strncmp(target,
					(char *)jVars_get(jVarJID),
					strlen(target))) {
				match = find_match(target);
				if (!match) {
					dprintf(dExecution, "No match\n");
					jwg_serverror(conn, "Unable to find a matching user.");

					break;
				}
			}
			else {
				match = strdup(target);
			}

			dprintf(dExecution, "To: %s\n", match);
			out = jabutil_pingnew(type, match);
			jab_send(jab_c, out);

			sprintf(retstr, "Successfully pinged %.67s.", match);
			jwg_servsuccess(conn, retstr);
		} break;

		case JWGPACKET_MESSAGE: {
			char *match, *target, *message, *dontmatch, *encrypt,
				*encmessage, *type;
			char retstr[80];
			xode out;

			encrypt = NULL;
			encmessage = NULL;
			target = xode_get_attrib(packet->x, "to");
			dontmatch = xode_get_attrib(packet->x, "dontmatch");
			encrypt = xode_get_attrib(packet->x, "encrypt");
			type = xode_get_attrib(packet->x, "type");
			if (!type) {
				type = strdup("chat");
			}

			dprintf(dExecution, "Received message\n");

			if (!target) {
				dprintf(dExecution, "No target specified.\n");
				jwg_serverror(conn, "No target specified.");
				break;
			}

			dprintf(dExecution, "Target %s\n", target);
			if (dontmatch && !strcmp(dontmatch, "yes")) {
				match = strdup(target);
			}
			else if (strncmp(target,
					(char *)jVars_get(jVarJID),
					strlen(target))) {
				match = find_match(target);
				if (!match) {
					dprintf(dExecution, "No match\n");
					jwg_serverror(conn, "Unable to find a matching user.");

					break;
				}
			}
			else {
				match = strdup(target);
			}

			message = xode_get_data(packet->x);
			if (!message) {
				message = "";
			}
			else {
				trim_message(message);
			}

#ifdef USE_GPGME
			if (encrypt) {
				char *keyid = JGetKeyID(match);

				if (!keyid) {
					dprintf(dExecution, "Unable to find keyid.\n");
					jwg_serverror(conn, "Unable to find a matching gpg key.");
					break;
				}

				encmessage = JEncrypt(message, keyid);
				if (!encmessage) {
					dprintf(dExecution, "Unable to encrypt message.\n");
					jwg_serverror(conn, "Unable to encrypt message.");
					break;
				}

				encmessage = JTrimPGPMessage(encmessage);
				if (!encmessage) {
					dprintf(dExecution, "Unable to trim encrypted message.\n");
					jwg_serverror(conn, "Unable to trim encrypted message.");
					break;
				}

				message = "[This message is encrypted.]";
			}
#endif /* USE_GPGME */

			dprintf(dExecution, "To: %s\nMsg: %s%s\n", match, message, encmessage ? " [encrypted]" : "");
			out = jabutil_msgnew(type, match, NULL, message, encmessage);
			jab_send(jab_c, out);

			sprintf(retstr, "Successfully sent to %.67s.", match);
			jwg_servsuccess(conn, retstr);
		} break;

		case JWGPACKET_LOCATE: {
			char *organization, *match, *showall, *target;
			int skipnotavail, strictmatch;

			dprintf(dExecution, "Received locate\n");

			target = xode_get_attrib(packet->x, "target");
			if (!target) {
				target = NULL;
			}

			strictmatch = 0;
			match = xode_get_attrib(packet->x, "match");
			if (match) {
				if (!strcmp(match, "strict")) {
					strictmatch = 1;
				}
			}

			skipnotavail = 1;
			showall = xode_get_attrib(packet->x, "showall");
			if (showall && !strcmp(showall, "yes")) {
				skipnotavail = 0;
			}

			organization = xode_get_attrib(packet->x, "organization");
			if (organization) {
				if (!strcmp(organization, "group")) {
					list_contacts_bygroup(jwg_c, target, strictmatch, skipnotavail);
				}
				else {
					list_contacts(jwg_c, target, strictmatch, skipnotavail);
				}
			}
			else {
				list_contacts(jwg_c, target, strictmatch, skipnotavail);
			}
		} break;

		case JWGPACKET_STATUS: {
			show_status();
		} break;

		case JWGPACKET_SHUTDOWN: {
			dprintf(dExecution, "Received shutdown\n");
			jwg_servsuccess(conn, "Shutdown initiated.");
			mux_end_loop_p = 1;
		} break;

		case JWGPACKET_CHECK: {
			dprintf(dExecution, "Received check\n");
			jwg_servsuccess(conn, "check");
		} break;

		case JWGPACKET_REREAD: {
			dprintf(dExecution, "Received reread\n");
			if (read_in_description_file()) {
				jwg_servsuccess(conn,
					"Description file loaded.");
			}
			else {
				jwg_serverror(conn,
				"Description file had an error.  Load failed.");
			}
		} break;

		case JWGPACKET_SHOWVAR: {
			xode out, x;
			char *var, *setting;

			dprintf(dExecution, "Received showvar\n");

			var = xode_get_attrib(packet->x, "var");
			if (!var) {
				jwg_serverror(conn, "Variable not specified.");
				break;
			}

			setting = jVars_show(jVars_stoi(var));
			if (!setting) {
				jwg_serverror(conn, jVars_get_error());
				break;
			}

			out = xode_new("results");
			x = xode_insert_cdata(out, setting, strlen(setting));
			jwg_servsend(conn, out);
			xode_free(out);
		} break;

		case JWGPACKET_SUBSCRIBE: {
			xode out, x, y;
			char *jid;

			dprintf(dExecution, "Received subscribe\n");

			jid = xode_get_attrib(packet->x, "jid");
			if (!jid) {
				jwg_serverror(conn, "JID not specified.");
				break;
			}
			dprintf(dExecution, "Subscribe: %s\n", jid);

			out = jabutil_presnew(JABPACKET__SUBSCRIBE, jid, "Jwgc Subscription Request", -1);
			jab_send(jab_c, out);
			xode_free(out);

			out = jabutil_iqnew(JABPACKET__SET, NS_ROSTER);
			x = xode_get_tag(out, "query");
			y = xode_insert_tag(x, "item");
			xode_put_attrib(y, "jid", jid);
			jab_send(jab_c, out);
			xode_free(out);

			out = xode_new("presence");
			xode_put_attrib(out, "to", jid);
			jab_send(jab_c, out);
			xode_free(out);

			jwg_servsuccess(conn, "Subscription successful.");
		} break;

		case JWGPACKET_UNSUBSCRIBE: {
			xode out, x, y;
			char *jid;

			dprintf(dExecution, "Received unsubscribe\n");

			jid = xode_get_attrib(packet->x, "jid");
			if (!jid) {
				jwg_serverror(conn, "JID not specified.");
				break;
			}
			dprintf(dExecution, "Unsubscribe: %s\n", jid);

			out = jabutil_presnew(JABPACKET__UNSUBSCRIBE, jid, "Jwgc Unsubscription Request", -1);
			jab_send(jab_c, out);
			xode_free(out);

			out = jabutil_iqnew(JABPACKET__SET, NS_ROSTER);
			x = xode_get_tag(out, "query");
			y = xode_insert_tag(x, "item");
			xode_put_attrib(y, "jid", jid);
			xode_put_attrib(y, "subscription", "remove");
			jab_send(jab_c, out);
			xode_free(out);

			jwg_servsuccess(conn, "Unsubscription successful.");
		} break;

		case JWGPACKET_NICKNAME: {
			xode out, x, y;
			char *jid, *nick;

			dprintf(dExecution, "Received nickname\n");

			jid = xode_get_attrib(packet->x, "jid");
			nick = xode_get_attrib(packet->x, "nick");
			if (!jid) {
				jwg_serverror(conn, "JID not specified.");
				break;
			}
			if (!nick) {
				jwg_serverror(conn, "Nickname not specified.");
				break;
			}
			dprintf(dExecution, "Nickname: %s -> %s\n", jid, nick);

			out = jabutil_iqnew(JABPACKET__SET, NS_ROSTER);
			x = xode_get_tag(out, "query");
			y = xode_insert_tag(x, "item");
			xode_put_attrib(y, "jid", jid);
			xode_put_attrib(y, "name", nick);
			jab_send(jab_c, out);
			xode_free(out);

			update_nickname(jid, nick);

			jwg_servsuccess(conn, "Nickname setting successful.");
		} break;

		case JWGPACKET_GROUP: {
			xode out, x, y, z, zz;
			char *jid, *group;

			dprintf(dExecution, "Received group\n");

			jid = xode_get_attrib(packet->x, "jid");
			group = xode_get_attrib(packet->x, "group");
			if (!jid) {
				jwg_serverror(conn, "JID not specified.");
				break;
			}
			if (!group) {
				jwg_serverror(conn, "Group not specified.");
				break;
			}
			dprintf(dExecution, "Group: %s -> %s\n", jid, group);

			out = jabutil_iqnew(JABPACKET__SET, NS_ROSTER);
			x = xode_get_tag(out, "query");
			y = xode_insert_tag(x, "item");
			xode_put_attrib(y, "jid", jid);
			z = xode_insert_tag(y, "group");
			zz = xode_insert_cdata(z, group, strlen(group));

			jab_send(jab_c, out);
			xode_free(out);

			update_group(jid, group);

			jwg_servsuccess(conn, "Group setting successful.");
		} break;

		case JWGPACKET_REGISTER: {
			xode out, query, queryout, x;
			char *jid, randnum[6];

			dprintf(dExecution, "Received register\n");

			jid = xode_get_attrib(packet->x, "jid");
			if (!jid) {
				jwg_serverror(conn, "JID not specified.");
				break;
			}
			dprintf(dExecution, "Register: %s\n", jid);

			query = xode_get_tag(packet->x, "query");
			if (query) {
				out = jabutil_iqnew(JABPACKET__SET, NS_REGISTER);
				queryout = xode_get_tag(out, "query");
			}
			else {
				out = jabutil_iqnew(JABPACKET__GET, NS_REGISTER);
			}
			sprintf(randnum, "%d", random() % 50000);
			xode_put_attrib(out, "id", randnum);
			xode_put_attrib(out, "to", jid);
			if (query) {
				x = xode_get_firstchild(query);
				while (x) {
					xode_insert_node(queryout, x);
					x = xode_get_nextsibling(x);
				}
			}
			jab_send(jab_c, out);
			xode_free(out);

			jab_c->dumpfd = conn->sckfd;
			jab_c->dumpid = randnum;
			jab_recv(jab_c);
			jab_c->dumpfd = -1;
			jab_c->dumpid = NULL;
		} break;

		case JWGPACKET_SEARCH: {
			xode out, query, queryout, x;
			char *jid, randnum[6];

			dprintf(dExecution, "Received search\n");

			jid = xode_get_attrib(packet->x, "jid");
			if (!jid) {
				jwg_serverror(conn, "JID not specified.");
				break;
			}
			dprintf(dExecution, "Search: %s\n", jid);

			query = xode_get_tag(packet->x, "query");
			if (query) {
				out = jabutil_iqnew(JABPACKET__SET, NS_SEARCH);
				queryout = xode_get_tag(out, "query");
			}
			else {
				out = jabutil_iqnew(JABPACKET__GET, NS_SEARCH);
			}
			sprintf(randnum, "%d", random() % 50000);
			xode_put_attrib(out, "id", randnum);
			xode_put_attrib(out, "to", jid);
			if (query) {
				x = xode_get_firstchild(query);
				while (x) {
					xode_insert_node(queryout, x);
					x = xode_get_nextsibling(x);
				}
			}
			jab_send(jab_c, out);
			xode_free(out);

			jab_c->dumpfd = conn->sckfd;
			jab_c->dumpid = randnum;
			jab_recv(jab_c);
			jab_c->dumpfd = -1;
			jab_c->dumpid = NULL;
		} break;

		case JWGPACKET_SETVAR: {
			xode out, x;
			char *var, *setting;

			dprintf(dExecution, "Received setvar\n");

			var = xode_get_attrib(packet->x, "var");
			if (!var) {
				jwg_serverror(conn, "Variable not specified.");
				break;
			}

			setting = xode_get_data(packet->x);
			if (!setting) {
				jwg_serverror(conn, "Empty setting.");
				break;
			}

			if (!jVars_set(jVars_stoi(var), setting)) {
				jwg_serverror(conn, jVars_get_error());
				break;
			}
			
			jwg_servsuccess(conn, "Variable successfully set.");
		} break;

		case JWGPACKET_JOIN: {
			xode out;
			char *chatroom;

			dprintf(dExecution, "Received join\n");
			chatroom = xode_get_attrib(packet->x, "room");
			if (!chatroom) {
				jwg_serverror(conn, "Target room not specified.");
				break;
			}

			out = jabutil_presnew(-1, chatroom, NULL, -1);
			jab_send(jab_c, out);

			jwg_servsuccess(conn, "Successfully joined chat room.");
		} break;

		case JWGPACKET_LEAVE: {
			xode out;
			char *chatroom;

			dprintf(dExecution, "Received leave\n");
			chatroom = xode_get_attrib(packet->x, "room");
			if (!chatroom) {
				jwg_serverror(conn, "Target room not specified.");
				break;
			}

			out = jabutil_presnew(JABPACKET__UNAVAILABLE, chatroom, NULL, -1);
			jab_send(jab_c, out);

			remove_from_contact_list(chatroom);

			jwg_servsuccess(conn, "Successfully left chat room.");
		} break;

		case JWGPACKET_DEBUG: {
			char *debugflags;

			dprintf(dExecution, "Received debug\n");
			debugflags = xode_get_tagdata(packet->x, "debugflags");
			if (!debugflags) {
				jwg_serverror(conn, "You must specify debugging flags.");
				break;
			}

			dparseflags(debugflags);

			jwg_servsuccess(conn, "Successfully adjusted debugging flags.");
		} break;

		default: {
			dprintf(dExecution, "JwgcEvent: unknown packet.\n");
		} break;
	}

	jwg_cleanup(conn);
	return;
}
