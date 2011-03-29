<?xml version='1.0' encoding="utf-8" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'
		             xmlns:date="http://exslt.org/dates-and-times">

<xsl:import href="ctl-cpp-common.xsl"/>
<xsl:output method="text" indent="yes" encoding="utf-8"/>

<xsl:variable name="CLASSNAME">
	<xsl:call-template name="settings"><xsl:with-param name="varname" select="'class-name'"/></xsl:call-template>
</xsl:variable>
<xsl:variable name="BASECLASS">
	<xsl:call-template name="settings"><xsl:with-param name="varname" select="'base-class'"/></xsl:call-template>
</xsl:variable>
<xsl:variable name="OID">
	<xsl:call-template name="settings"><xsl:with-param name="varname" select="'ID'"/></xsl:call-template>
</xsl:variable>

<!-- Генерирование cc-файла -->
<xsl:template match="/">

<!-- BEGIN CC-FILE -->
// Метод с использованием заказа датчиков
// --------------------------------------------------------------------------
<xsl:call-template name="COMMON-CC-HEAD"/>
// --------------------------------------------------------------------------
<xsl:call-template name="COMMON-CC-FUNCS"/>
// --------------------------------------------------------------------------
<xsl:call-template name="COMMON-CC-FILE"/>
// --------------------------------------------------------------------------
void <xsl:value-of select="$CLASSNAME"/>_SK::callback()
{
	if( !active )
		return;
	try
	{	
		isTestMode = checkTestMode();
		if( trTestMode.change(isTestMode) )
			testMode(isTestMode);

		if( isTestMode )
		{
			if( !active )
				return;

			msleep( sleep_msec );
			return;
		}

		// проверка таймеров
		checkTimers(this);

		if( resetMsgTime&gt;0 &amp;&amp; trResetMsg.hi(ptResetMsg.checkTime()) )
		{
//			cout &lt;&lt; myname &lt;&lt;  ": ********* reset messages *********" &lt;&lt; endl;
			resetMsg();
		}

		// обработка сообщений (таймеров и т.п.)
		for( int i=0; i&lt;<xsl:call-template name="settings"><xsl:with-param name="varname" select="'msg-count'"/></xsl:call-template>; i++ )
		{
			if( !receiveMessage(msg) )
				break;
			processingMessage(&amp;msg);
			updateOutputs(false);
			updatePreviousValues();
		}

		// Выполнение шага программы
		step();

		// "сердцебиение"
		if( idHeartBeat!=DefaultObjectId &amp;&amp; ptHeartBeat.checkTime() )
		{
			ui.saveValue(idHeartBeat,maxHeartBeat,UniversalIO::AnalogInput);
			ptHeartBeat.reset();
		}

		// обновление выходов
		updateOutputs(false);
		updatePreviousValues();
	}
	catch( Exception&amp; ex )
	{
		unideb[Debug::CRIT] &lt;&lt; myname &lt;&lt; "(execute): " &lt;&lt; ex &lt;&lt; endl;
	}
	catch(CORBA::SystemException&amp; ex)
	{
		unideb[Debug::CRIT] &lt;&lt; myname &lt;&lt; "(execute): СORBA::SystemException: "
			&lt;&lt; ex.NP_minorString() &lt;&lt; endl;
	}
	catch(...)
	{
		unideb[Debug::CRIT] &lt;&lt; myname &lt;&lt; "(execute): catch ..." &lt;&lt; endl;
	}

	if( !active )
		return;
	
	msleep( sleep_msec );
}
// -----------------------------------------------------------------------------
void <xsl:value-of select="$CLASSNAME"/>_SK::setValue( UniSetTypes::ObjectId _sid, long _val )
{
//	ui.setState(sid,state);
	<xsl:for-each select="//smap/item">
		<xsl:if test="normalize-space(@vartype)='out'">
		if( _sid == <xsl:value-of select="@name"/> )
		{
			unideb[Debug::LEVEL2] &lt;&lt;  "(setState): <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = " &lt;&lt;  _val &lt;&lt;  endl;
			<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>	= _val;
				<xsl:call-template name="setdata"/>
			return;
		}
		</xsl:if>
		<xsl:if test="normalize-space(@vartype)='io'">
		if( _sid == <xsl:value-of select="@name"/> )
		{
			unideb[Debug::LEVEL2] &lt;&lt;  "(setState): <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = " &lt;&lt;  _val &lt;&lt;  endl;
			<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>	= _val;
				<xsl:call-template name="setdata"/>
			return;
		}
		</xsl:if>
	</xsl:for-each>
}
// -----------------------------------------------------------------------------
void <xsl:value-of select="$CLASSNAME"/>_SK::updateOutputs( bool _force )
{
	<xsl:for-each select="//smap/item">
	if( _force || prev_<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> != <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> )
	{
		<xsl:choose>
		<xsl:when test="normalize-space(@vartype)='out'"><xsl:call-template name="setdata"/></xsl:when>
		<xsl:when test="normalize-space(@vartype)='io'"><xsl:call-template name="setdata"/></xsl:when>
		</xsl:choose>
		prev_<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>;
	}
	</xsl:for-each>

<!--
// update messages
<xsl:for-each select="//msgmap/item">
	if( prev_m_<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> != m_<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> )
	{
		si.id = <xsl:value-of select="@name"/>;
		ui.saveState( si,m_<xsl:value-of select="@name"/>,UniversalIO::DigitalInput,getId() );
		prev_m_<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = m_<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>;		
	}
</xsl:for-each>
-->
}
// -----------------------------------------------------------------------------
void <xsl:value-of select="$CLASSNAME"/>_SK::preSensorInfo( UniSetTypes::SensorMessage* _sm )
{
	<xsl:for-each select="//smap/item">
	<xsl:if test="normalize-space(@vartype)='in'">
	if( _sm->id == <xsl:value-of select="@name"/> )
			<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = _sm->value;
	</xsl:if>
	<xsl:if test="normalize-space(@vartype)='io'">
	if( _sm->id == <xsl:value-of select="@name"/> )
			<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = _sm->value;
	</xsl:if>
	</xsl:for-each>
	
	sensorInfo(_sm);
}
// -----------------------------------------------------------------------------
void <xsl:value-of select="$CLASSNAME"/>_SK::askState( UniSetTypes::ObjectId _sid, UniversalIO::UIOCommand _cmd, UniSetTypes::ObjectId _node )
{
	ui.askRemoteSensor(_sid,_cmd,_node,getId());
}
// -----------------------------------------------------------------------------
void <xsl:value-of select="$CLASSNAME"/>_SK::askValue( UniSetTypes::ObjectId _sid, UniversalIO::UIOCommand _cmd, UniSetTypes::ObjectId _node )
{
	ui.askRemoteSensor(_sid,_cmd,_node,getId());
}
// -----------------------------------------------------------------------------
bool <xsl:value-of select="$CLASSNAME"/>_SK::getState( UniSetTypes::ObjectId _sid )
{
<xsl:for-each select="//smap/item">
	if( _sid == <xsl:value-of select="@name"/> &amp;&amp; <xsl:value-of select="@name"/> != DefaultObjectId )
	{
	<xsl:choose>
	<xsl:when test="normalize-space(@iotype)='DI'">
		<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = ui.getState(<xsl:value-of select="@name"/>, node_<xsl:value-of select="@name"/>);
		return <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>;
	</xsl:when>
	<xsl:when test="normalize-space(@iotype)='DI'">
		<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = ui.getState(<xsl:value-of select="@name"/>, node_<xsl:value-of select="@name"/>);
		return <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>;
	</xsl:when>
	<xsl:when test="normalize-space(@iotype)='AO'">
		<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = ui.getValue(<xsl:value-of select="@name"/>, node_<xsl:value-of select="@name"/>);
		return <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>;
	</xsl:when>
	<xsl:when test="normalize-space(@iotype)='AI'">
		<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = ui.getValue(<xsl:value-of select="@name"/>, node_<xsl:value-of select="@name"/>);
		return <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>;
	</xsl:when>
	</xsl:choose>
	}
</xsl:for-each>
	unideb[Debug::CRIT] &lt;&lt; myname &lt;&lt; "(getState): Обращение к неизвестному ДИСКРЕТНОМУ датчику sid="
		&lt;&lt; _sid &lt;&lt; endl;
	
	return false;
}
// -----------------------------------------------------------------------------

void <xsl:value-of select="$CLASSNAME"/>_SK::askSensors( UniversalIO::UIOCommand _cmd )
{
	PassiveTimer ptAct(activateTimeout);
	while( !activated &amp;&amp; !ptAct.checkTime() )
	{	
		cout &lt;&lt; myname &lt;&lt; "(askSensors): wait activate..." &lt;&lt; endl;
		msleep(300);
		if( activated )
			break;
	}
			
	if( !activated )
		unideb[Debug::CRIT] &lt;&lt; myname 
			&lt;&lt; "(askSensors): ************* don`t activated?! ************" &lt;&lt; endl;

	for( ;; )
	{
		try
		{
		<xsl:for-each select="//smap/item">
		<xsl:if test="normalize-space(@vartype)='in'">
			if( <xsl:value-of select="@name"/> != DefaultObjectId )
				ui.askRemoteSensor(<xsl:value-of select="@name"/>,_cmd,node_<xsl:value-of select="@name"/>,getId());
		</xsl:if> 
		<xsl:if test="normalize-space(@vartype)='io'">
			if( <xsl:value-of select="@name"/> != DefaultObjectId )
				ui.askRemoteSensor(<xsl:value-of select="@name"/>,_cmd,node_<xsl:value-of select="@name"/>,getId());
		</xsl:if> 
		</xsl:for-each>
			return;
		}
		catch(SystemError&amp; err)
		{
			unideb[Debug::CRIT] &lt;&lt; myname &lt;&lt; "(askSensors): " &lt;&lt; err &lt;&lt; endl;
		}
		catch(Exception&amp; ex)
		{
			unideb[Debug::CRIT] &lt;&lt; myname &lt;&lt; "(askSensors): " &lt;&lt; ex &lt;&lt; endl;
		}
		catch(...)
		{
			unideb[Debug::CRIT] &lt;&lt; myname &lt;&lt; "(askSensors): catch(...)" &lt;&lt; endl;
		}
		msleep(askPause);
	}
}
// -----------------------------------------------------------------------------
void <xsl:value-of select="$CLASSNAME"/>_SK::setMsg( UniSetTypes::ObjectId _code, bool _state )
{
	// блокируем сброс (т.к. он автоматически по таймеру)
	if( !_state )
	{
		ptResetMsg.reset();
		return; 
	}

	alarm( _code, _state );
	ptResetMsg.reset();
}	
// ----------------------------------------------------------------------------
<!-- END CC-FILE -->
</xsl:template>

<xsl:template name="getdata">
<xsl:param name="output">0</xsl:param>	
	if( <xsl:value-of select="@name"/> != DefaultObjectId )
	{
		<xsl:choose>
			<xsl:when test="normalize-space(@iotype)='DI'">
				<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = ui.getState(<xsl:value-of select="@name"/>, node_<xsl:value-of select="@name"/>);
			</xsl:when>
			<xsl:when test="normalize-space(@iotype)='AI'">
				<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = ui.getValue(<xsl:value-of select="@name"/>, node_<xsl:value-of select="@name"/>);
			</xsl:when>
		</xsl:choose>
		<xsl:choose>
			<xsl:when test="normalize-space(@iotype)='DO'">
				<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = ui.getState(<xsl:value-of select="@name"/>, node_<xsl:value-of select="@name"/>);
			</xsl:when>
			<xsl:when test="normalize-space(@iotype)='AO'">
				<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> = ui.getValue(<xsl:value-of select="@name"/>, node_<xsl:value-of select="@name"/>);
			</xsl:when>
		</xsl:choose>
	}
</xsl:template>

<xsl:template name="setdata">
		if( <xsl:value-of select="@name"/> != DefaultObjectId )
		{
		<xsl:choose>
		<xsl:when test="normalize-space(@iotype)='DI'">
			si.id 	= <xsl:value-of select="@name"/>;
			si.node = node_<xsl:value-of select="@name"/>;
			ui.saveState( si, <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>,UniversalIO::DigitalInput,getId() );
		</xsl:when>
		<xsl:when test="normalize-space(@iotype)='DO'">
			ui.setState( <xsl:value-of select="@name"/>, <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>, node_<xsl:value-of select="@name"/> );
		</xsl:when>
		<xsl:when test="normalize-space(@iotype)='AI'">
			si.id 	= <xsl:value-of select="@name"/>;
			si.node = node_<xsl:value-of select="@name"/>;
			ui.saveValue( si, <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>,UniversalIO::AnalogInput, getId() );
		</xsl:when>
		<xsl:when test="normalize-space(@iotype)='AO'">
			ui.setValue( <xsl:value-of select="@name"/>, <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>, node_<xsl:value-of select="@name"/> );
		</xsl:when>
		</xsl:choose>
		}
</xsl:template>

<xsl:template name="setdata_value">
<xsl:param name="setval">0</xsl:param>	
	if( <xsl:value-of select="@name"/> != DefaultObjectId )
	{
	<xsl:choose>
		<xsl:when test="normalize-space(@iotype)='DI'">
		si.id 	= <xsl:value-of select="@name"/>;
		si.node = node_<xsl:value-of select="@name"/>;
		ui.saveState( si,<xsl:value-of select="$setval"/>,UniversalIO::DigitalInput,getId() );
		</xsl:when>
		<xsl:when test="normalize-space(@iotype)='DO'">
		ui.setState( <xsl:value-of select="@name"/>,<xsl:value-of select="$setval"/>, node_<xsl:value-of select="@name"/>);
		</xsl:when>
		<xsl:when test="normalize-space(@iotype)='AI'">
		si.id 	= <xsl:value-of select="@name"/>;
		si.node = node_<xsl:value-of select="@name"/>;
		ui.saveValue( si,<xsl:value-of select="$setval"/>,UniversalIO::AnalogInput, getId() );
		</xsl:when>
		<xsl:when test="normalize-space(@iotype)='AO'">
		ui.setValue( <xsl:value-of select="@name"/>,<xsl:value-of select="$setval"/>, node_<xsl:value-of select="@name"/> );
		</xsl:when>
	</xsl:choose>
	}
</xsl:template>


<xsl:template name="check_changes">
<xsl:param name="onlymsg"></xsl:param>	
<xsl:choose>
	<xsl:when test="normalize-space(@iotype)='DI'">
		<xsl:if test="normalize-space($onlymsg)=''">
		if( prev_<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> != <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> )
		</xsl:if>
		{
			if( <xsl:value-of select="@name"/> != DefaultObjectId )
			{
			<xsl:if test="normalize-space($onlymsg)=''">		
//				cout &lt;&lt; myname &lt;&lt; ": (DI) change state <xsl:value-of select="@name"/> set " 
//						&lt;&lt; <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> &lt;&lt; endl;
			</xsl:if>
				SensorMessage _sm( <xsl:value-of select="@name"/>, (bool)<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>, Message::Medium);
				_sm.sensor_type = UniversalIO::DigitalInput;
				sensorInfo(&amp;_sm);
			}
		}
	</xsl:when>
	<xsl:when test="normalize-space(@iotype)='AI'">
		<xsl:if test="normalize-space($onlymsg)=''">
		if( prev_<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> != <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> )
		</xsl:if>
		{
			if( <xsl:value-of select="@name"/> != DefaultObjectId )
			{
			<xsl:if test="normalize-space($onlymsg)=''">
//				cout &lt;&lt; myname &lt;&lt; ": (AI) change value <xsl:value-of select="@name"/> set " 
//						&lt;&lt; <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> &lt;&lt; endl;
			</xsl:if>
				// приходится искуственно использовать третий параметр, 
				// что-бы компилятор выбрал
				// правильный(для аналоговых) конструктор у SensorMessage
				IOController_i::CalibrateInfo _ci;
				SensorMessage _sm( <xsl:value-of select="@name"/>, (long)<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>, _ci);
				_sm.sensor_type = UniversalIO::AnalogInput;
				sensorInfo(&amp;_sm);
			}
		}
	</xsl:when>
	<xsl:when test="normalize-space(@iotype)='DO'">
		<xsl:if test="normalize-space($onlymsg)=''">
		if( prev_<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> != <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> )
		</xsl:if>
		{
			if( <xsl:value-of select="@name"/> != DefaultObjectId )
			{
			<xsl:if test="normalize-space($onlymsg)=''">
//				cout &lt;&lt; myname &lt;&lt; ": (DO) change state <xsl:value-of select="@name"/> set " 
//						&lt;&lt; <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> &lt;&lt; endl;
			</xsl:if>
				SensorMessage _sm( <xsl:value-of select="@name"/>, (bool)<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>, Message::Medium);
				_sm.sensor_type = UniversalIO::DigitalOutput;
				sensorInfo(&amp;_sm);
			}
		}
	</xsl:when>
	<xsl:when test="normalize-space(@iotype)='AO'">
		<xsl:if test="normalize-space($onlymsg)=''">
		if( prev_<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> != <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> )
		</xsl:if>
		{
			if( <xsl:value-of select="@name"/> != DefaultObjectId )
			{
			<xsl:if test="normalize-space($onlymsg)=''">
//				cout &lt;&lt; myname &lt;&lt; ": (AO) change value <xsl:value-of select="@name"/> set " 
//						&lt;&lt; <xsl:call-template name="setprefix"/><xsl:value-of select="@name"/> &lt;&lt; endl;
			</xsl:if>
				// приходится искуственно использовать третий параметр, 
				// что-бы компилятор выбрал
				// правильный(для аналоговых) конструктор у SensorMessage
				IOController_i::CalibrateInfo _ci;
				SensorMessage _sm( <xsl:value-of select="@name"/>, (long)<xsl:call-template name="setprefix"/><xsl:value-of select="@name"/>, _ci);
				_sm.sensor_type = UniversalIO::AnalogOutput;
				sensorInfo(&amp;_sm);
			}
		}
	</xsl:when>
</xsl:choose>
</xsl:template>

</xsl:stylesheet>
