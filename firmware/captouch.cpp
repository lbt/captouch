//============================================================
//	Touch Sensing Demo on Spark Core
//
//============================================================
//	Copyright (c) 2014 Tangibit Studios LLC.  All rights reserved.
//  Copyright (c) 2014 David Greaves <david@dgreaves.com>
//
//	This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//	License as published by the Free Software Foundation, either
//	version 3 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public
//	License along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//============================================================
#include "application.h"
#include "captouch.h"

// This has to be accessible by the ISR until the ISR supports lambdas
unsigned long CapTouch_tR = 0;

CapTouch::CapTouch(int sensorPin, int driverPin) :
     m_intrIsAttached(false)
    ,m_pollTime(CAPTOUCH_POLL_TIME)
{
    m_driverPin = driverPin;
    m_sensorPin = sensorPin;
    m_touchSenseLast = LOW;
    m_touchDebounceTimeLast = 0;
    m_touchNow = LOW;
    m_touchLast = LOW;
}

void CapTouch::detachIntr() {
    detachInterrupt(m_sensorPin);
    m_intrIsAttached = false;
}

void CapTouch::attachIntr() {
//    auto mylambda = [&]() {this->touchSense();} ;
//    attachInterrupt(m_sensorPin,  (void (*)())mylambda , RISING);
    attachInterrupt(m_sensorPin,  touchSense , RISING);
    m_intrIsAttached = true;
}

void CapTouch::setup()
{
    // intialize conditions for touch sensor
    pinMode(m_driverPin,OUTPUT);
    attachIntr();
    // calibrate touch sensor- Keep hands off!!!
    m_tBaseline = touchSampling();    // initialize to first reading
	// time stamps
	m_lastUpdate = 0;
    
}

CapTouch::Event CapTouch::getEvent()
{
    CapTouch::Event touchEvent = CapTouch::NoEvent;
	//Update every POLL_TIME [ms]
	if (millis() > m_lastUpdate + m_pollTime)
	{
		// check Touch UI
		touchEvent = touchEventCheck();
        #ifdef CAPTOUCH_DEBUG
        Serial.print("check at ");
 		Serial.print(m_lastUpdate);
 		#endif

		// time stamp updated
		m_lastUpdate = millis();
	}
	return(touchEvent);
}

//------------------------------------------------------------
// ISR for touch sensing
//------------------------------------------------------------
void CapTouch::touchSense()
{
    CapTouch_tR = micros();
}

//------------------------------------------------------------
// touch sampling
//
// sample touch sensor 32 times and get average RC delay [usec]
//------------------------------------------------------------
long CapTouch::touchSampling()
{
    long tDelay = 0;
    int mSample = 0;
    
    for (int i=0; i<32; i++)
    {
        // discharge capacitance at rPin
        pinMode(m_sensorPin, OUTPUT);
        digitalWrite(m_driverPin,LOW);
        digitalWrite(m_sensorPin,LOW);
        
        // revert to high impedance input
        pinMode(m_sensorPin,INPUT);
        
        // timestamp & transition sPin to HIGH and wait for interrupt in a read loop
        m_tS = micros();
        CapTouch_tR = m_tS;
        digitalWrite(m_driverPin,HIGH);
        do
        {
            // wait for transition - interrupt handler will set tR
        } while (digitalRead(m_sensorPin)==LOW);
        
        // accumulate the RC delay samples
        // ignore readings when micros() overflows
        if (CapTouch_tR>m_tS)
        {
            tDelay = tDelay + (CapTouch_tR - m_tS);
            mSample++;
        }
        
    }
    
    // calculate average RC delay [usec]
    if (mSample>0)
    {
        tDelay = tDelay/mSample;
    }
    else
    {
        tDelay = 0;     // this is an error condition!
    }

    //autocalibration using exponential moving average on data below trigger point
    if (tDelay<(m_tBaseline + m_tBaseline/4))
    {
        m_tBaseline = m_tBaseline + (tDelay - m_tBaseline)/8;
    }
    #ifdef CAPTOUCH_DEBUG
	Serial.print(" Delay:baseline=");
	Serial.print(tDelay);
	Serial.print(":");
	Serial.print(m_tBaseline);
	#endif

    return tDelay;
    
}

//------------------------------------------------------------
// touch event check
//
// check touch sensor for events:
//      NoEvent       no change
//      TouchEvent    sensor is touched (Low to High)
//      ReleaseEvent  sensor is released (High to Low)
//
//------------------------------------------------------------
CapTouch::Event CapTouch::touchEventCheck()
{
    unsigned long touchSense;                             // current reading
    unsigned long touchDebounceTime = 50;                 // debounce time
    CapTouch::Event tEvent = CapTouch::NoEvent; // default event
    
    // read touch sensor
    m_tReading = touchSampling();
    
    // touch sensor is HIGH if trigger point 1.25*Baseline
    if (m_tReading>(m_tBaseline + m_tBaseline/4)) {
        touchSense = HIGH; 
    } else {
        touchSense = LOW; 
    }
    
    // debounce touch sensor
    // if state changed then reset debounce timer
    if (touchSense != m_touchSenseLast) {
        m_touchDebounceTimeLast = millis();
    }
    
    m_touchSenseLast = touchSense;
    
    // accept as a stable sensor reading if the debounce time is exceeded without reset
    if (millis() > m_touchDebounceTimeLast + touchDebounceTime) {
        m_touchNow = touchSense;
    }
    
    
    // set events based on transitions between readings
    if (!m_touchLast && m_touchNow) {
        tEvent = TouchEvent;
    }
    
    if (m_touchLast && !m_touchNow) {
        tEvent = ReleaseEvent;
    }
    
    // update last reading
    m_touchLast = m_touchNow;
    #ifdef CAPTOUCH_DEBUG
    Serial.print("  Event:");
    if (tEvent == TouchEvent)
        Serial.println(" Touch");
    else if (tEvent == ReleaseEvent)
        Serial.println(" Release");
    else if (tEvent == NoEvent)
        Serial.println(" None");
	#endif
	
    return tEvent;
}
