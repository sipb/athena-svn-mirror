package org.GNOME.Speech;
import org.GNOME.Bonobo.*;
import org.GNOME.Speech.*;
import org.GNOME.Accessibility.*;
import org.omg.CORBA.*;
import java.util.Vector;
import java.io.*;
import org.GNOME.Bonobo.*;
import com.sun.speech.freetts.Voice;
import com.sun.speech.freetts.audio.JavaClipAudioPlayer;

public class FreeTTSSpeaker extends UnknownImpl implements SpeakerOperations {

	private FreeTTSSynthesisDriver driver;
	private SpeechCallback cb = null;
	public Voice v;
	private float rate;
	private float pitch;
	private float volume;
	private boolean refresh = true;

	public FreeTTSSpeaker (FreeTTSSynthesisDriver driver,
			       Voice v) {
		poa = JavaBridge.getRootPOA ();
		tie = new SpeakerPOATie (this, poa);
		try {
			poa.activate_object(tie);
		} catch (Exception e) {
			System.err.println ("error: " + e);
		}
		this.driver = driver;
		this.v = v;
		driver.ref ();

		/* Cache current speech parameters */

		pitch = v.getPitch();
		rate = v.getRate();
		volume = v.getVolume();
	}
	
	public org.GNOME.Speech.Parameter[] getSupportedParameters () {
		Parameter[] parameters = new Parameter[3];
		parameters[0] = new Parameter ("pitch",0.0, pitch, 1000, false);
		parameters[1] = new Parameter ("rate", 0.0, rate, 999, false);
		parameters[2] = new Parameter ("volume", 0.0, volume, 100.0, false);
		return parameters;
	}
	
	public String getParameterValueDescription (String name, double value) {
		return "";
	}

	public double getParameterValue (String name) {
		if (name.equals("pitch")) {
			return (double) pitch;
		}
		if (name.equals("rate")) {
			return (double) rate;
		}
		if (name.equals("volume")) {
			return (double) volume * 100.0;
		}
		return 0.0;
	}

	public boolean setParameterValue (String name, double value) {
		if (name.equals("pitch")) {
			pitch = (float) value;
		}
		if (name.equals("rate")) {
			rate = (float) value;
		}
		if (name.equals("volume")) {
			volume = (float) ( value / 100.0 );
		}
		refresh = true;
		return true;
	}

	/* Speech functions */
	
	public int say (String text) {
		return driver.say (this, text);
	}

	public boolean stop () {
		return driver.stop ();
	}
    
	public boolean isSpeaking () {
		return driver.isSpeaking ();
	}

	public void _wait () {
	}

        /* Speech callbacks */

	public boolean registerSpeechCallback (org.GNOME.Speech.SpeechCallback callback) {
		cb = callback;
		return true;
	}

	public float getPitch () {
		return pitch;
	}

	public float getRate () {
		return rate;
	}

	public float getVolume () {
		return volume;
	}

	public SpeechCallback getCallback () {
		return cb;
	}

	public boolean needsParameterRefresh () {
		return refresh;
	}
	
	public void clearParameterRefresh () {
		refresh = false;
	}
		
}
