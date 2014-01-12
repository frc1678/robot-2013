class Toggler
{
public:
	bool output; // output of toggle, on or off?
	bool listenForToggle; // are we waiting to toggle?
	bool recentChange; // has the toggle value recently changed?
	
	//for two-button toggles. a bit of amisnomer, but this allows the same object to do many things
	bool listenForOnToggle;
	bool recentOnToggleChange;
	bool listenForOffToggle;
	bool recentOffToggleChange;
	Toggler()
	{
		output = false;
		listenForToggle = true;
		recentChange = false;
		listenForOnToggle = true;
		recentOnToggleChange = false;
		listenForOffToggle = true;
		recentOffToggleChange = false;
	}
	
	void toggle(bool input) // input is the button press, usually
	{
		// If the button is pressed and we're waiting to toggle
		if(input && listenForToggle)
		{
			//toggle and stop waiting to toggle
			output = !output;
			listenForToggle = false;
			recentChange = true;
		}
		// Set it to no recent change if you hold down the button
		else if(input && !listenForToggle)
		{
			recentChange = false;
		}
		
		
		//If we're not pressing the button, then we're waiting to toggle.
		if(!input)
		{
			listenForToggle = true;
			recentChange = false;
		}
	}
	
	void toggleOn(bool input)
	{
		// If the button is pressed and we're waiting to toggle
		if(input && listenForOnToggle)
		{
			//set to true and stop waiting to toggle
			output = true;
			listenForOnToggle = false;
			recentOnToggleChange = true;
		}
		// Set it to no recent change if you hold down the button
		else if(input && !listenForOnToggle)
		{
			recentOnToggleChange = false;
		}
		
		
		//If we're not pressing the button, then we're waiting to toggle.
		if(!input)
		{
			listenForOnToggle = true;
			recentOnToggleChange = false;
		}
		
	}
	
	void toggleOff(bool input)
	{
		// If the button is pressed and we're waiting to toggle
		if(input && listenForOffToggle)
		{
			//set to false and stop waiting to toggle
			output = false;
			listenForOffToggle = false;
			recentOffToggleChange = true;
		}
		// Set it to no recent change if you hold down the button
		else if(input && !listenForOffToggle)
		{
			recentOffToggleChange = false;
		}
		
		
		//If we're not pressing the button, then we're waiting to toggle.
		if(!input)
		{
			listenForOffToggle = true;
			recentOffToggleChange = false;
		}
		
	}
	
	// Indicates the status of the toggler
	bool status()
	{
		return output;
	}
	
	// Has the toggler recently changed value
	bool isRecentlyChanged()
	{
		return recentChange;
	}
	
	// Manually change the return value
	// Can be used in a "semi-toggle"
	void Set(bool setTo)
	{
		output = setTo;
	}
	
	// Reset the togglers, for example in an init.
	void disable()
	{
		output = false;
		listenForToggle = true;
	}
};
