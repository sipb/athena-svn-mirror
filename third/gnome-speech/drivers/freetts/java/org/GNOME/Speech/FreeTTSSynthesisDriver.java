package org.GNOME.Speech;
import org.GNOME.Bonobo.*;
import org.GNOME.Speech.*;
import org.GNOME.Accessibility.*;
import org.omg.CORBA.*;
import java.util.Vector;
import java.util.Locale;
import java.io.*;
import com.sun.speech.freetts.Voice;
import com.sun.speech.freetts.VoiceManager;
import com.sun.speech.freetts.Gender;
import com.sun.speech.freetts.audio.JavaClipAudioPlayer;

public class FreeTTSSynthesisDriver extends UnknownImpl implements SynthesisDriverOperations {

	private boolean initialized = false;
	private VoiceManager vm = null;
	private Voice[] voices = null;
	private int index = 0;
	private SpeechThread thread = null;
	
	
	public void bonoboActivate (String iid, String [] args) {
		ORB orb = AccessUtil.getORB ();
		String ior = orb.object_to_string (tie._this_object());
		_bonoboActivate (ior, iid, args);
	}

	public native void _bonoboActivate (String ior, String iid,
					    String [] arggs);

	static {
		System.loadLibrary ("freettsdriver");
	}
    
	public FreeTTSSynthesisDriver (String [] args) {
		poa = JavaBridge.getRootPOA ();
		tie = new SynthesisDriverPOATie (this, poa);
		try {
			poa.activate_object(tie);
			bonoboActivate ("OAFIID:GNOME_Speech_SynthesisDriver_FreeTTS:proto0.3", args);
		} catch (Exception e) {
			System.err.println ("error: " + e);
		}
	}

        /* Descriptive attributes */

	public String driverName () {
		return "FreeTTS Gnome Speech Driver";
	}

	public String synthesizerName () {
		return "FreeTTS";
	}

	public String driverVersion () {
		return "0.3";
	}
    
	public String synthesizerVersion () {
		return "1.2";
	}

	/* Initialization */

	public boolean driverInit () {
		if (initialized)
			return true;
		vm = VoiceManager.getInstance ();
		thread = new SpeechThread();
		thread.start ();
		initialized = true;
		return true;
	}

	public boolean isInitialized () {
		return initialized;
	}	

	/* Voice related functions */

	private VoiceInfo getVoiceInfo (Voice v) {
		Gender g = v.getGender ();
		VoiceInfo vi = new VoiceInfo ();

		vi.name = v.getName ();
		if (g == Gender.MALE)
			vi.gender = voice_gender.gender_male;
		else if (g == Gender.FEMALE)
			vi.gender = voice_gender.gender_female;
		Locale l = v.getLocale ();
		vi.language = l.getLanguage() + "_" + l.getCountry ();
		return vi;
	}

        private boolean voiceMatchesSpec (Voice voice, String specLocale) {
	    String lang = specLocale;
	    int index = specLocale.indexOf ('_');
	    Locale locale = voice.getLocale ();

	    System.out.println (specLocale);

	    // TODO: should we just use String.matches (Regexp) here instead ?

	    if (index > 0) lang = specLocale.substring (0, index); 
	    if (locale.getLanguage().equals (lang)) {
		String country = null;
		String voiceCountry = locale.getCountry ();
		if (index > 0) {
		    country = specLocale.substring (++index);
		    index = specLocale.indexOf ('@');
		    if (index > 0) {
			country = country.substring (0, index);
			return false; // For now, FreeTTS doesn't have any locales with encoding suffices
		    }
		}
		if ((country == null) || (voiceCountry == null) || (voiceCountry.length () == 0) || 
		    voiceCountry.toUpperCase().equals (country)) 
		    return true;
	    }
	    return false;
	}

	public org.GNOME.Speech.VoiceInfo[] getVoices (org.GNOME.Speech.VoiceInfo voice_spec) {
		int i;
		int num_voices;
		Vector matched = new Vector ();
		VoiceInfo vi = null;
		VoiceInfo via[];

		System.err.println ("getVoices...");

		System.err.println ("spec name=" + voice_spec.name + " lang=" + voice_spec.language);

		if (voices == null)
			voices = vm.getVoices ();
		num_voices = voices.length;
		for (i = 0; i < num_voices; i++)
		{
			vi = getVoiceInfo (voices[i]);

		    System.err.println ("checking voice " + i + " " + vi.name);
			
			// If the name isn't empty and doesn't match, skip this one

			if (!voices[i].getDomain().equals("general"))
				continue;
			if (voice_spec.name.length () != 0 && !voice_spec.name.equals(vi.name))
				continue;
			if (voice_spec.language.length () != 0 && !voiceMatchesSpec (voices[i], voice_spec.language))
				continue;
			if (((voice_spec.gender == voice_gender.gender_female) || 
			     (voice_spec.gender == voice_gender.gender_male)) && 
                             // would test gender_unspecified, but API freeze blocks addition to enum...
			    voice_spec.gender != vi.gender)
				continue;
			System.err.println ("adding speaker " + vi.name);
			matched.add (vi);
		}
		num_voices = matched.size ();
		via = new VoiceInfo[num_voices];
		for (i = 0; i < num_voices; i++)
		{
			via[i] = (VoiceInfo) matched.remove (0);
		}
		return via;
	}

	public org.GNOME.Speech.VoiceInfo[] getAllVoices () {
		int i;
		int num_voices;
		VoiceInfo[] vi;
		Vector list = new Vector ();

		System.out.println ("getAllVoices...");

		if (voices == null)
			voices = vm.getVoices ();

		num_voices = voices.length;
		for (i = 0; i < num_voices; i++)
		{
			if (!voices[i].getDomain().equals("general"))
				continue;
			list.add(getVoiceInfo(voices[i]));
		}
		num_voices = list.size ();
		vi = new VoiceInfo[num_voices];
		for (i = 0; i < num_voices; i++)
		{
			vi[i] = (VoiceInfo) list.remove (0);
		}
		return vi;
	}

	/* Speaker creation */

	public org.GNOME.Speech.Speaker createSpeaker (org.GNOME.Speech.VoiceInfo voice_spec) {
		Speaker speaker = null;
		VoiceInfo vi;
		int i;
		int num_voices;

		if (voices == null)
			voices = vm.getVoices ();

		num_voices = voices.length;
		for (i = 0; i < num_voices; i++)
		{
			if (voices[i].getDomain () != "general")
				continue;
			vi = getVoiceInfo (voices[i]);
			if (voice_spec.name.length () != 0 && !voice_spec.name.equals(vi.name))
				continue;
			if (voice_spec.language.length () != 0 && !voice_spec.language.equals(vi.language))
				continue;
			
			System.out.println ("Found voice.");
			if (!voices[i].isLoaded ())
			{
				System.out.println ("Loading voice.");
				voices[i].setAudioPlayer (new JavaClipAudioPlayer());
				voices[i].allocate ();
			}
			speaker = SpeakerHelper.narrow ((new FreeTTSSpeaker (this, voices[i])).tie());
			break;
		}
		return speaker;
	}

	public static void main (String [] args) {
		System.out.println ("Hello, FreeTTS Driver is not running.");
		try {
			FreeTTSSynthesisDriver driver = new FreeTTSSynthesisDriver (args);	    
			System.out.println ("ORB running");
			AccessUtil.getORB().run();
		} catch (Exception e) {
			System.err.println ("error: " + e);
		}
	}
	
	public int say (FreeTTSSpeaker s,
			String text) {
		index++;
		thread.addEntry (s, index, text);
		return index;
	}
	
	public boolean stop () {
		thread.removeAll ();
		return true;
	}

	public boolean isSpeaking () {
		return true;
	}

	class QueueEntry {

		public int index;
		public String text;
		FreeTTSSpeaker s = null;

		public QueueEntry (FreeTTSSpeaker s, 
				   int index,
				   String text) {
			this.s = s;
			this.text = text;
			this.index = index;
		}
		
	}
	
	class SpeechThread extends Thread {
		private boolean done = false;
		private Vector SpeechQueue = null;
		private QueueEntry current = null;
	
		public SpeechThread () {
			SpeechQueue = new Vector ();
		}
	
		public void run () {
			current = null;
			while (done == false) {
				synchronized (SpeechQueue) {
					if (SpeechQueue.size() == 0) {
						try {
							SpeechQueue.wait ();
						} catch (InterruptedException ie) {
						}
					}
				}
				synchronized (SpeechQueue) {
					current = (QueueEntry) SpeechQueue.remove(0);
				}
				if (current != null) {
					
					/* Set speaker parameters */

					if (current.s.needsParameterRefresh()) {
						current.s.v.setPitch (current.s.getPitch());
						current.s.v.setRate(current.s.getRate());
						current.s.v.setVolume(current.s.getVolume());
					}
					if (current.s.getCallback() != null)
						try {
							current.s.getCallback()._notify (speech_callback_type.speech_callback_speech_started,
										   current.index, 0);
						} catch (Exception ex) {
							System.err.println ("error: " + ex);
						}

					System.out.println ("speaking " + current.text);
					current.s.v.speak (current.text);
					if (current.s.getCallback() != null)
						try {
							current.s.getCallback()._notify (speech_callback_type.speech_callback_speech_ended,
										   current.index, 0);
						} catch (Exception ex) {
							System.err.println ("error: " + ex);
						}
					
				}
			}
		}
	
		public void addEntry (FreeTTSSpeaker s,
				      int index,
				      String text) {
			QueueEntry e = new QueueEntry (s, index, text);
			synchronized (SpeechQueue) {
				SpeechQueue.add(e);
				SpeechQueue.notifyAll();
			}
		}

		public synchronized void quit () {
			removeAll ();
			done = true;
		}

		public synchronized void removeAll () {
			current.s.v.getAudioPlayer().cancel();
			synchronized (SpeechQueue) {
				SpeechQueue.removeAllElements();
			}
		}
	}
}
