using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/*Binaural Audio Source. Use this on each audio source you want to perform binaural effect with.*/
public class BinauralAudioSource : MonoBehaviour 
{
	/*Path of the audio file.*/
	public string audioPath = @"C:\test2.mp3";
	/*An unique id used to identify the audio source object.*/
	public string sourceID = "1";
	/*Specify the hrtf effect.*/
	public bool hrtf = true;

	void Start () 
	{
		BinauralEngine.AddAudioSource (audioPath, sourceID);
		BinauralEngine.SetAudioSourcePos (sourceID, transform.position.x, transform.position.y, transform.position.z);
		BinauralEngine.PlayAudioSource (sourceID);
	}
		
	void Update () 
	{
		BinauralEngine.SetAudioSourceHRTF (sourceID, hrtf);
		BinauralEngine.SetAudioSourcePos (sourceID, transform.position.x, transform.position.y, transform.position.z);
	}

	void OnEnable()
	{
		BinauralEngine.SetAudioSourcePaused (sourceID, false);
	}

	void OnDisable()
	{
		BinauralEngine.SetAudioSourcePaused (sourceID, true);
	}

	void OnDestroy()
	{
		BinauralEngine.StopAudioSource (sourceID);
		BinauralEngine.RemoveAudioSource (sourceID);
	}

	void OnApplicationPause(bool paused)
	{
		BinauralEngine.SetAudioSourcePaused (sourceID, paused);
	}
}
