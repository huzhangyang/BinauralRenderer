using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Runtime.InteropServices;  

/*Main Audio Engine. Use this on audio listener uniquely.*/
public class BinauralEngine : MonoBehaviour 
{
	/*Must specify an matlab bin library to use*/
	public string matlabPath = @"C:\Program Files\MATLAB\R2016b\bin\win64";

	/*Must specify a CIPIC hrir file to use*/
	public string hrtfPath = @"C:\test.mat";

	/*public functions*/
	[DllImport("BinauralRender")]  
	public static extern void AddAudioSource (string filename, string sourceID);
	[DllImport("BinauralRender")]  
	public static extern void RemoveAudioSource (string sourceID);
	[DllImport("BinauralRender")]  
	public static extern void PlayAudioSource(string sourceID);
	[DllImport("BinauralRender")]  
	public static extern void StopAudioSource(string sourceID);
	[DllImport("BinauralRender")]  
	public static extern void SetAudioSourcePaused(string sourceID, bool paused);
	[DllImport("BinauralRender")]  
	public static extern bool IsAudioSourcePlaying(string sourceID);
	[DllImport("BinauralRender")]  
	public static extern void SetAudioSourceHRTF(string sourceID, bool enable);
	[DllImport("BinauralRender")]  
	public static extern void SetAudioSourcePos(string sourceID, float posx, float posy, float posz);

	/*private functions*/
	[DllImport("BinauralRender")]  
	private static extern void UpdateAudioEngine ();
	[DllImport("BinauralRender")]  
	private static extern void ReleaseAudioEngine ();
	[DllImport("BinauralRender")]  
	private static extern void SetHRIR(string filename);
	[DllImport("BinauralRender")]  
	private static extern void SetListener(float posx, float posy, float posz, float orix, float oriy, float oriz);

	void Awake()
	{		
		String currentPath = Environment.GetEnvironmentVariable("PATH");
		if(currentPath.Contains(matlabPath) == false)
		{
			Environment.SetEnvironmentVariable("PATH", Environment.GetEnvironmentVariable("PATH") + ";" + matlabPath);
		}
	}

	void Start () 
	{
        SetHRIR(hrtfPath);
    }
		
	void Update () 
	{
		SetListener(transform.position.x, transform.position.y, transform.position.z,
			transform.rotation.eulerAngles.x, transform.rotation.eulerAngles.y, transform.rotation.eulerAngles.z);
		UpdateAudioEngine();
	}

	void OnDestroy()
	{
		ReleaseAudioEngine ();
	}

	void OnApplicationQuit()
	{
		ReleaseAudioEngine ();
	}
}
