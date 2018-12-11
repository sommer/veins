///*******************************************************************************
//* @author  Joseph Kamel
//* @email   josephekamel@gmail.com
//* @date    28/11/2018
//* @version 1.1
//*
//* SCA (Secure Cooperative Autonomous systems)
//* Copyright (c) 2013, 2018 Institut de Recherche Technologique SystemX
//* All rights reserved.
//*******************************************************************************/

package org.car2x.veins.modules.application.f2mdVeinsApp;
import org.car2x.veins.modules.application.f2mdVeinsApp.BaseWaveApplLayer;

//
// network description file for your Veins Application. Add parameters here
//
simple JosephVeinsApp extends BaseWaveApplLayer
{
    @class(JosephVeinsApp);
    string appName = default("Joseph Mb Veins App");
    string helloWorld = default("testInput");
}
