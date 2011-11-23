<#if protocolName=="CSMA 802.15.4">
<?xml version="1.0" encoding="UTF-8"?>
<root>
	<Decider type="Decider802154Narrow">
		<!--Length of Start Frame Delimiter (used to compute probability of successful 
			synchronization)-->
		<parameter name="sfdLength" type="long" value="8"/>
		
		<!--minimum possible bit error rate (BER floor)-->
		<parameter name="berLowerBound" type="double" value="1e-8"/>
		
		<!--modulation type-->
		<parameter name="modulation" type="string" value="oqpsk16"/>
	</Decider>
</root>
</#if>
