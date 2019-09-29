package ru.lewa_j.nenuzhno.engine;

public class JNILib
{
	static
	{
		System.loadLibrary("nenuzhno-engine");
	}

	public static native void created();
	public static native void changed(int w, int h);
	public static native void draw();
	public static native void ontouch(float x, float y, int a);
	public static native void onkey(int k, int a);
}
