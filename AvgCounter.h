/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/
//copied and modified from WPILib

#ifndef CPPAVGCOUNTER_H_
#define CPPAVGCOUNTER_H_

#include "AnalogTriggerOutput.h"
#include "CounterBase.h"
#include "SensorBase.h"
#include "LiveWindow/LiveWindowSendable.h"

/**
 * Class for counting the number of ticks on a digital input channel.
 * This is a general purpose class for counting repetitive events. It can return the number
 * of counts, the period of the most recent cycle, and detect when the signal being counted
 * has stopped by supplying a maximum cycle time.
 */
class AvgCounter : public SensorBase, public CounterBase, public LiveWindowSendable
{
public:
	typedef enum {kTwoPulse=0, kSemiperiod=1, kPulseLength=2, kExternalDirection=3} Mode;

	AvgCounter();
	explicit AvgCounter(UINT32 channel);
	AvgCounter(UINT8 moduleNumber, UINT32 channel);
	explicit AvgCounter(DigitalSource *source);
	explicit AvgCounter(DigitalSource &source);
	explicit AvgCounter(AnalogTrigger *trigger);
	explicit AvgCounter(AnalogTrigger &trigger);
	AvgCounter(EncodingType encodingType, DigitalSource *upSource, DigitalSource *downSource, bool inverted);
	virtual ~AvgCounter();

	void SetUpSource(UINT32 channel);
	void SetUpSource(UINT8 moduleNumber, UINT32 channel);
	void SetUpSource(AnalogTrigger *analogTrigger, AnalogTriggerOutput::Type triggerType);
	void SetUpSource(AnalogTrigger &analogTrigger, AnalogTriggerOutput::Type triggerType);
	void SetUpSource(DigitalSource *source);
	void SetUpSource(DigitalSource &source);
	void SetUpSourceEdge(bool risingEdge, bool fallingEdge);
	void ClearUpSource();

	void SetDownSource(UINT32 channel);
	void SetDownSource(UINT8 moduleNumber, UINT32 channel);
	void SetDownSource(AnalogTrigger *analogTrigger, AnalogTriggerOutput::Type triggerType);
	void SetDownSource(AnalogTrigger &analogTrigger, AnalogTriggerOutput::Type triggerType);
	void SetDownSource(DigitalSource *source);
	void SetDownSource(DigitalSource &source);
	void SetDownSourceEdge(bool risingEdge, bool fallingEdge);
	void ClearDownSource();

	void SetUpDownCounterMode();
	void SetExternalDirectionMode();
	void SetSemiPeriodMode(bool highSemiPeriod);
	void SetPulseLengthMode(float threshold);

	void SetReverseDirection(bool reverseDirection);

	// CounterBase interface
	void Start();
	INT32 Get();
	void Reset();
	void Stop();
	double GetPeriod();
	void SetMaxPeriod(double maxPeriod);
	void SetUpdateWhenEmpty(bool enabled);
	bool GetStopped();
	bool GetDirection();
	UINT32 GetIndex() {return m_index;}
	
	
	void UpdateTable();
	void StartLiveWindowMode();
	void StopLiveWindowMode();
	virtual std::string GetSmartDashboardType();
	void InitTable(ITable *subTable);
	ITable * GetTable();

	// FPGA addition: (stuff we added)
	void SetSamplesToAverage(int samplesToAverage);
	int GetSamplesToAverage();
	
private:
	void InitCounter(Mode mode = kTwoPulse);

	DigitalSource *m_upSource;		///< What makes the counter count up.
	DigitalSource *m_downSource;	///< What makes the counter count down.
	bool m_allocatedUpSource;		///< Was the upSource allocated locally?
	bool m_allocatedDownSource;	///< Was the downSource allocated locally?
	tCounter *m_counter;				///< The FPGA counter object.
	UINT32 m_index;					///< The index of this counter.
	
	ITable *m_table;
};

#endif
