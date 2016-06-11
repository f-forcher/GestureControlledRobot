package franzf.robotcontroller;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.objdetect.CascadeClassifier;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;
import at.abraxas.amarino.Amarino;

public class RobotControlActivity extends Activity implements CvCameraViewListener2 {

    private static final String    TAG                 = "OCVSample::Activity";
    private static final Scalar    FACE_RECT_COLOR     = new Scalar(0, 255, 0, 255);
    public static final int        JAVA_DETECTOR       = 0;
    public static final int        NATIVE_DETECTOR     = 1;
    public static final Scalar	   BACKGROUND_COLOR    = new Scalar(0, 0, 0, 255);
    public static final Scalar	   COLORE_BIANCO    = new Scalar(255, 255, 255, 255);
    public static final String	   BT_ADDRESS 		   = "00:13:EF:00:07:AE"; 

    private MenuItem               mItemFace50;
    private MenuItem               mItemFace40;
    private MenuItem               mItemFace30;
    private MenuItem               mItemFace20;
    private MenuItem               mItemType;

    private Mat                    mRgba;
    private Mat                    mGray;
	
    //Questa Mtemp l'ho aggiunta io.
    private Mat                    mTemp;
    
    private File                   mCascadeFile;
    private CascadeClassifier      mJavaDetector;
    //private DetectionBasedTracker  mNativeDetector;

    private int                    mDetectorType       = JAVA_DETECTOR;
    private String[]               mDetectorName;

    private float                  mRelativeFaceSize   = 0.2f;
    private int                    mAbsoluteFaceSize   = 0;
    
    //MIE VARIABILI
    private static final Point mOrigin = new Point(0,0);
    private  Point mCorner;
    private Point nuovoPuntoMirror1;
    private Point nuovoPuntoMirror2;
    private Point puntoMedio;
    
    //Array contenente 2 valori da 0 a 255, il primo è per il motore destro e il secondo per il sinistro
    private int[] velMotori = new int[2];
    private double yc=0;
    private double xc=0;
    private int yn=0;
    private int xn=0;
    
    private boolean vediFotoFlag = false;
    

    private CameraBridgeViewBase   mOpenCvCameraView;

    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    //System.loadLibrary("detection_based_tracker");

                    try {
                        // carica il file cascade
                    	// TODO: L'ESEMPIO ORIGINALE USAVA LBPCASCADE_FRONTALFACE!!!!
                        InputStream is = getResources().openRawResource(R.raw.agest);
                        File cascadeDir = getDir("cascade", Context.MODE_PRIVATE);
                        mCascadeFile = new File(cascadeDir, "lbpcascade_frontalface.xml");
                        FileOutputStream os = new FileOutputStream(mCascadeFile);

                        byte[] buffer = new byte[4096];
                        int bytesRead;
                        while ((bytesRead = is.read(buffer)) != -1) {
                            os.write(buffer, 0, bytesRead);
                        }
                        is.close();
                        os.close();

                        mJavaDetector = new CascadeClassifier(mCascadeFile.getAbsolutePath());
                        if (mJavaDetector.empty()) {
                            Log.e(TAG, "Failed to load cascade classifier");
                            mJavaDetector = null;
                        } else
                            Log.i(TAG, "Loaded cascade classifier from " + mCascadeFile.getAbsolutePath());

                        cascadeDir.delete();

                    } catch (IOException e) {
                        e.printStackTrace();
                        Log.e(TAG, "Failed to load cascade. Exception thrown: " + e);
                    }

                    mOpenCvCameraView.enableView();
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };

    public RobotControlActivity() {
        mDetectorName = new String[2];
        mDetectorName[JAVA_DETECTOR] = "Java";
        mDetectorName[NATIVE_DETECTOR] = "Native (tracking)";

        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    /** Chiamato quando viene crata l'activity*/
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.robot_controller_layout);

        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.fd_activity_surface_view);
        mOpenCvCameraView.setCvCameraViewListener(this);
        
        Amarino.connect(this, BT_ADDRESS);
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
    }

    public void onDestroy() {
        super.onDestroy();
        mOpenCvCameraView.disableView();
        
        //Disconnetti amarino
        Amarino.disconnect(this, BT_ADDRESS);
    }

    public void onCameraViewStarted(int width, int height) {
        mGray = new Mat();
        mRgba = new Mat();
        mTemp = new Mat();
    }

    public void onCameraViewStopped() {
        mGray.release();
        mRgba.release();
        mTemp.release();
    }

    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {

        mRgba = inputFrame.rgba();
        mGray = inputFrame.gray();
        int height = mGray.rows();
        int width = mGray.cols();
        mCorner = new Point(width , height);

        if (mAbsoluteFaceSize == 0) {;
            if (Math.round(height * mRelativeFaceSize) > 0) {
                mAbsoluteFaceSize = Math.round(height * mRelativeFaceSize);
            }
        }

        MatOfRect faces = new MatOfRect();
        
        if (mDetectorType == JAVA_DETECTOR) {
            if (mJavaDetector != null)
                mJavaDetector.detectMultiScale(mGray, faces, 1.1, 2, 2,
                        new Size(mAbsoluteFaceSize, mAbsoluteFaceSize), new Size());
        }
        else if (mDetectorType == NATIVE_DETECTOR) {
            /*if (mNativeDetector != null)
                mNativeDetector.detect(mGray, faces);*/
				//NOn uso il NativeDetector
        }
        else {
            Log.e(TAG, "Detection method is not selected!");
        }
        
        Rect[] facesArray = faces.toArray();
       
        //Dipinge un immagine nera per coprire l'anteprima della foto/**/
        if (!vediFotoFlag) {
			Core.rectangle(mRgba, mOrigin, mCorner, new Scalar(0, 0, 0, 255), -3);
		}
        /**/
        
        //Disegna la zona morta
        Core.circle(mRgba, new Point(width/2,height/2), (int)(height/(float)6), COLORE_BIANCO,2);
        
        //Ignora i dati se ci sono più faccie o nessuna, prima c'era un ciclo for
		if (facesArray.length==1) {
				//Modificato per fare simmetria speculare TODO:FINIRE AMARINO!
				
				nuovoPuntoMirror1 = new Point(width-facesArray[0].tl().x,facesArray[0].tl().y);
				nuovoPuntoMirror2 = new Point(width-facesArray[0].br().x,facesArray[0].br().y);
				
				//calcola il punto medio nel sistema standard di riferimento (origine in alto a sinistra, credo)
				puntoMedio = new Point((nuovoPuntoMirror1.x+nuovoPuntoMirror2.x)/2,(nuovoPuntoMirror1.y+nuovoPuntoMirror2.y)/2);
				//puntoMedio;
				/*
				 * v1=yc+xc =====> Velocità motore SINISTRO!
				 * v2=yc-xc =====> Velocità motore DESTRO!
				 * Sistema di riferimento centrale (origine=centro tablet e asse y verso l'alto):
				 * yc=-(y-height/2) e xc=(x-width/2)
				 * Normalizzati (e poi arrontondati) con yn=(yc/height)*255 e xn=(xc/width)*255 cosicchè xn e yn variano da 0 a 255
				 * (per es yc/height è la percentuale dell'asse y raggiunta, dato che height è l'altezza massima)
				 */
				xc=(puntoMedio.x-width/2);
				yc=0-(puntoMedio.y-height/2);
				
				//Disegna il rettangolo della mano con le diagonali. Il centro è il pundo che dà i dati;
				Core.rectangle(mRgba, nuovoPuntoMirror1, nuovoPuntoMirror2, FACE_RECT_COLOR, 3);
				Core.line(mRgba, nuovoPuntoMirror1, nuovoPuntoMirror2, FACE_RECT_COLOR, 3);
				Core.line(mRgba, new Point(nuovoPuntoMirror1.x,nuovoPuntoMirror2.y), new Point(nuovoPuntoMirror2.x,nuovoPuntoMirror1.y), FACE_RECT_COLOR, 3);
				
				Log.w(TAG, "Mano rilevata! Inviato: M"+velMotori[0]+";"+velMotori[1]);
				
				//Definisci la zona morta come un cerchio di raggio 16.66% di height centrato nell'origine di (xc;yc)
				if (xc*xc+yc*yc<( ((float)height)/6)*(((float)height)/6) ){
					velMotori[0]=0;
					velMotori[1]=0;
					Amarino.sendDataToArduino(this, BT_ADDRESS, 'm', velMotori);
				}
				else {
					//100 perchè si arriva a 100 e poi si aggiunge 180, sennò il robot non si muove
					yn=(int)Math.round((yc/height)*127);
					xn=(int)Math.round((xc/width)*127);

					//Se sarebbe positivo si aggiunge 154, se negativo si sottrae
					velMotori[0]=restringi((yn+xn)+(int)(Math.signum(yn+xn)*180),-255,255);
					velMotori[1]=restringi((yn-xn)+(int)(Math.signum(yn-xn)*180),-255,255);

					Amarino.sendDataToArduino(this, BT_ADDRESS, 'm', velMotori);
				}
		} else {
		//Ferma i motori se non rilevo alcuna mano
			velMotori[0]=0;
			velMotori[1]=0;
			Amarino.sendDataToArduino(this, BT_ADDRESS, 'm', velMotori);
		}
		return mRgba;
        /**/
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.i(TAG, "called onCreateOptionsMenu");
        mItemFace50 = menu.add("Dimensione mani 50%");
        mItemFace40 = menu.add("Dimensione mani 40%");
        mItemFace30 = menu.add("Dimensione mani 30%");
        mItemFace20 = menu.add("Dimensione mani 20%");
        mItemType   = menu.add("Mostra immagine");
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Log.i(TAG, "called onOptionsItemSelected; selected item: " + item);
        if (item == mItemFace50)
            setMinFaceSize(0.5f);
        else if (item == mItemFace40)
            setMinFaceSize(0.4f);
        else if (item == mItemFace30)
            setMinFaceSize(0.3f);
        else if (item == mItemFace20)
            setMinFaceSize(0.2f);
        else if (item == mItemType) {
        	vediFotoFlag=!vediFotoFlag;
        }
        return true;
    }

    private void setMinFaceSize(float faceSize) {
        mRelativeFaceSize = faceSize;
        mAbsoluteFaceSize = 0;
    }

    private void setVediFotoFlag(boolean setFlag) {
    	vediFotoFlag=setFlag;
    }
    
    //Analogo di constrain di Arduino
    private static final int restringi(int x, int min, int max){
    	if(x<=min)
    		return min;
    	else if(x>=max)
    		return max;
    	else
    		return x;
    }
}
