<?xml version="1.0" encoding="UTF-8"?>
<root>
	<AnalogueModels>
		<AnalogueModel type="SimplePathlossModel">
	    	<parameter name="alpha" type="double" value="3.0"/>
	    	<parameter name="carrierFrequency" type="double" value="2.412e+9"/>
	    </AnalogueModel>
	</AnalogueModels>
<#if protocolName=="802.11">
	<Decider type="Decider80211">
		<!-- SNR threshold [NOT dB]-->
		<parameter name="threshold" type="double" value="0.12589254117942"/>
		
		<!-- The center frequency on whcih the phy listens-->
		<parameter name="centerFrequency" type="double" value="2.412e9"/>
	</Decider>
<#else>	
	<Decider type="SNRThresholdDecider">
		<!-- SNR threshold -->
		<parameter name="threshold" type="double" value="0.12589254117942"/>
	</Decider>
</#if>
</root>