package ru.lewa_j.nenuzhno.engine;

import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import javax.microedition.khronos.opengles.*;
import javax.microedition.khronos.egl.*;
import android.view.*;

public class WrapperRenderer implements Renderer
{
	GLSurfaceView glView;
	public WrapperRenderer(GLSurfaceView gv)
	{
		glView = gv;
	}

	@Override
	public void onSurfaceCreated(GL10 oldGL, EGLConfig config)
	{
		JNILib.created();
	}

	@Override
	public void onSurfaceChanged(GL10 oldGL, int w, int h)
	{
		JNILib.changed(w, h);
	}

	@Override
	public void onDrawFrame(GL10 oldGL)
	{
		JNILib.draw();
	}

	public void onTouchEvent(MotionEvent event)
	{
		float tx = event.getX();
		float ty = event.getY();
		int ta = event.getAction();
		JNILib.ontouch(tx,ty,ta);
	}

	public void onKey(int k, int a)
	{
		JNILib.onkey(k, a);
	}
}
