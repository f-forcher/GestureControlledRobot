package franzf.robotcontrollertattile;

import java.util.StringTokenizer;

import android.os.Bundle;
import android.preference.PreferenceManager;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.ImageView;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Switch;
import android.widget.TextView;
import at.abraxas.amarino.Amarino;
import at.abraxas.amarino.AmarinoIntent;

public class RobotControllerTattileActivity extends Activity implements OnSeekBarChangeListener, SensorEventListener, RadioGroup.OnCheckedChangeListener {
	//Costanti
	private static final String DEVICE_ADDRESS = "00:13:EF:00:07:AE";
	long lastChange;
	//Intervallo tra gli invii di dati
	final int DELAY = 200;

	private ArduinoReceiver arduinoReceiver = new ArduinoReceiver();
	private SensorManager mSensorManager;
	private Sensor mOrientation;

	//View
	TextView velocitaTxVw;
	TextView motSinTxVw;
	TextView motDesTxVw;
	TextView sensTxVw;

	SeekBar velSB;
	RadioGroup marciaSelector;
	Switch switchFanali;
	Switch switchFrenoAMano;
	Switch switchAltoparlante;

	ImageView imageVentola;
	ImageView imageAltoparlante;
	ImageView imageFanali;
	ImageView imageAutobrake;

	//Variabili
	float velTot=0;
	Float[] sensoreDestro = {0.0f,0.0f};
	Float[] sensoreSinistro = {0.0f,0.0f};
	Integer sensoreProssimita = 0;

	int vMax=0;
	int marcia=1;

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_robot_controller_tattile);

		velocitaTxVw = (TextView)  findViewById(R.id.textViewVelocita);
		motSinTxVw = (TextView)  findViewById(R.id.textViewMotSinistro);
		motDesTxVw = (TextView)  findViewById(R.id.textViewMotDestro);
		sensTxVw = (TextView)  findViewById(R.id.textViewSensoreVal);

		switchFanali = (Switch) findViewById(R.id.switchFanali);
		switchAltoparlante = (Switch) findViewById(R.id.switchAltoparlante);

		velSB = (SeekBar) findViewById(R.id.seekBarVelocita);
		marciaSelector = (RadioGroup) findViewById(R.id.radioGroup1);

		imageVentola = (ImageView) findViewById(R.id.imageViewVentola);
		imageFanali = (ImageView) findViewById(R.id.imageViewFanali);
		imageAltoparlante = (ImageView) findViewById(R.id.imageViewAltoparlante);
		imageAutobrake = (ImageView) findViewById(R.id.imageViewAutobrake);

		mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
		mOrientation = mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);

		velSB.setOnSeekBarChangeListener(this);
		marciaSelector.setOnCheckedChangeListener(this);
	}

	protected void onStart() {
		super.onStart();
		// in order to receive broadcasted intents we need to register our receiver
		registerReceiver(arduinoReceiver, new IntentFilter(AmarinoIntent.ACTION_RECEIVED));
		velSB.setProgress(0);
		imageVentola.setVisibility(View.INVISIBLE);
		imageFanali.setVisibility(View.INVISIBLE);
		imageAltoparlante.setVisibility(View.INVISIBLE);
		imageAutobrake.setVisibility(View.INVISIBLE);

		// this is how you tell Amarino to connect to a specific BT device from within your own code
		Amarino.connect(this, DEVICE_ADDRESS);
	}

	@Override
	protected void onStop() {
		super.onStop();

		unregisterReceiver(arduinoReceiver);
		// stop Amarino's background service, we don't need it any more 
		Amarino.disconnect(this, DEVICE_ADDRESS);
	}

	protected void onPause() {
		super.onPause();
		mSensorManager.unregisterListener(this);
	}


	protected void onResume() {
		super.onResume();
		mSensorManager.registerListener(this, mOrientation, SensorManager.SENSOR_DELAY_NORMAL);
	}


	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_robot_controller_tattile,
				menu);
		return true;
	}


	/**
	 * ArduinoReceiver riceve gli Events inviati da Amarino
	 */
	public class ArduinoReceiver extends BroadcastReceiver {

		@Override
		public void onReceive(Context context, Intent intent) {
			String dataString = null;
			String flagString = null;
			char flagChar='z';
			StringTokenizer dataStringTk = null;

			Log.w("Robot Controller Tattile: ","Ricevuti dati");

			// Indirizzo del device che invia i dati
			final String address = intent.getStringExtra(AmarinoIntent.EXTRA_DEVICE_ADDRESS);

			// tipo di dati
			final int dataType = intent.getIntExtra(AmarinoIntent.EXTRA_DATA_TYPE, -1);

			// controlla se il tipo di dati è una stringa
			if (dataType == AmarinoIntent.STRING_EXTRA){
				dataString = intent.getStringExtra(AmarinoIntent.EXTRA_DATA);

				if (dataString != null){

					//Togli la flag all'inizio (es "S20;15;16" diventa "20;15;16")
					flagString = dataString.substring(0, 1);
					flagChar = flagString.charAt(0);
					dataString = dataString.substring(1);

					switch (flagChar){

					case 'S':
						dataStringTk = new StringTokenizer(dataString,";");

						sensoreProssimita = Integer.parseInt(dataStringTk.nextToken());
						
						//Rudimentale filtro low-pass per i dati dei sensori di corrente, per evitare i problemi da noise e PWM
						sensoreDestro[0]=sensoreDestro[1];
						sensoreSinistro[0]=sensoreSinistro[1];
						
						sensoreDestro[1] = (Integer.parseInt(dataStringTk.nextToken()))*5.317f;
						sensoreSinistro[1] = (Integer.parseInt(dataStringTk.nextToken()))*5.317f;

						sensTxVw.setText(sensoreProssimita.toString());
						//Cast da float a Float per poter fare toString
						motDesTxVw.setText(( (Float) ((sensoreDestro[0]+sensoreDestro[1])/2.0f)).toString());
						motSinTxVw.setText(( (Float) ((sensoreSinistro[0]+sensoreSinistro[1])/2.0f)).toString());

						if (sensoreProssimita<13) {
							imageAutobrake.setVisibility(View.VISIBLE);
						} else {
							imageAutobrake.setVisibility(View.INVISIBLE);
						}

						break;
					case 'v':
						if(Integer.parseInt(dataString)==1)
							imageVentola.setVisibility(View.VISIBLE);
						else 
							imageVentola.setVisibility(View.INVISIBLE);
					}
					//velocitaTxVw.setText(dataString);

					/*dataStringTk = new StringTokenizer(dataString,";");

					sensoreDestro = Integer.parseInt(dataStringTk.nextToken());
					sensoreSinistro = Integer.parseInt(dataStringTk.nextToken());

					velocitaTxVw.setText("Sensori: "+sensoreSinistro+";"+sensoreDestro);*/
				}
			}
		}
	}


	@Override
	public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		if (System.currentTimeMillis() - lastChange > DELAY ){

			//TODO: Finire
			/*int[] velArray = {progress,progress};
			Amarino.sendDataToArduino(this, DEVICE_ADDRESS, 'm', velArray);*/

			vMax=progress;
			lastChange = System.currentTimeMillis();
		}

	}

	public void onFanali(View view){
		// Is the toggle on?
		boolean on = ((Switch) view).isChecked();

		if (on) {
			Amarino.sendDataToArduino(this, DEVICE_ADDRESS, 'f', 1);
			imageFanali.setVisibility(View.VISIBLE);

		} else {
			Amarino.sendDataToArduino(this, DEVICE_ADDRESS, 'f', 0);
			imageFanali.setVisibility(View.INVISIBLE);
		}
	}

	public void onAltoparlante(View view){
		// Is the toggle on?
		boolean on = ((Switch) view).isChecked();

		if (on) {
			Amarino.sendDataToArduino(this, DEVICE_ADDRESS, 'a', 1);
			imageAltoparlante.setVisibility(View.VISIBLE);

		} else {
			Amarino.sendDataToArduino(this, DEVICE_ADDRESS, 'a', 0);
			Amarino.sendDataToArduino(this, DEVICE_ADDRESS, 'a', 0);
			imageAltoparlante.setVisibility(View.INVISIBLE);
		}
	}
	
	public void onFreno(View view){
				
	}
	@Override
	public void onStartTrackingTouch(SeekBar seekBar) {
		lastChange = System.currentTimeMillis();

	}

	@Override
	public void onStopTrackingTouch(SeekBar seekBar) {
		// TODO Auto-generated method stub

	}

	@Override
	public void onAccuracyChanged(Sensor arg0, int arg1) {
		// TODO Auto-generated method stub

	}

	@Override
	public void onSensorChanged(SensorEvent event) {
		float azimuth_angle = event.values[0];
		float pitch_angle = event.values[1];
		float roll_angle = event.values[2];
		int vDestro=0;
		int vSinistro=0;
		int[] velArray;

		velocitaTxVw.setText("P: "+pitch_angle);

		if (System.currentTimeMillis() - lastChange > DELAY ) {

			//Controlla la marcia
			switch (marcia) {
			//inverto i motori...
			case 1:
				if (Math.signum(pitch_angle) <= 0) {
					vDestro = vMax;
					vSinistro = Math.round((1.0f - (pitch_angle / 90.0f)) * vMax);
				} else {
					vSinistro = vMax;
					vDestro = Math.round((Math.abs(1.0f - (pitch_angle) / 90.0f))
							* vMax);
				}
				velArray =  new int[]{ vDestro, vSinistro };
				Amarino.sendDataToArduino(this, DEVICE_ADDRESS, 'm', velArray);
				break;

			case -1:
				if (Math.signum(pitch_angle) <= 0) {
					vDestro = vMax;
					vSinistro = Math.round((1.0f - (pitch_angle / 90.0f)) * vMax);
				} else {
					vSinistro = vMax;
					vDestro = Math.round((Math.abs(1.0f - (pitch_angle) / 90.0f))
							* vMax);
				}
				velArray = new int[]{ -vDestro, -vSinistro };
				Amarino.sendDataToArduino(this, DEVICE_ADDRESS, 'm', velArray);
				break;

			case 0:
				if (Math.signum(pitch_angle) <= 0) {
					vDestro = Math.round((pitch_angle / 90.0f)*255);
					vSinistro = -vDestro;
				} else {
					vSinistro = -Math.round(Math.abs((pitch_angle) / 90.0f)* 255);
					vDestro = vSinistro;
				}
				velArray = new int[]{ vDestro, vSinistro };
				Amarino.sendDataToArduino(this, DEVICE_ADDRESS, 'm', velArray);
				break;

			}
			lastChange = System.currentTimeMillis();
		}
	}

	@Override
	public void onCheckedChanged(RadioGroup group, int checkedId) {
		switch (checkedId){
		case R.id.radioMarciaAvanti:
			marcia=1;
			break;
		case R.id.radioMarciaRetro:
			marcia=-1;
			break;
		case R.id.radioMarciaNeutra:
			marcia=0;
			break;
		}
	}

}
