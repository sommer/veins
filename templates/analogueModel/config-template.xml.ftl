<?xml version="1.0" encoding="UTF-8"?>
<root>
	<AnalogueModels>
	
		<AnalogueModel type="${amName}">
		
			<#if par1Name!="">
			<!--TODO: explain what this parameter does-->
	    	<parameter name="${par1Name}" type="${par1Type}" value=<#if par1Type=="bool">"false"<#else>"0"</#if>/>
			</#if>
			
			<#if par2Name!="">
			<!--TODO: explain what this parameter does-->
	    	<parameter name="${par2Name}" type="${par2Type}" value=<#if par2Type=="bool">"false"<#else>"0"</#if>/>
			</#if>
	    	
	    </AnalogueModel>
	    
	</AnalogueModels>
</root>