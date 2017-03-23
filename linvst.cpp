/*
 * linvst is based on wacvst Copyright 2009 retroware. All rights reserved. and dssi-vst Copyright 2004-2007 Chris Cannam
 
linvst Mark White 2017

	This file is part of linvst.

    linvst is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "remotevstclient.h"

extern "C" {

#define VST_EXPORT   __attribute__ ((visibility ("default")))

    extern VST_EXPORT AEffect * VSTPluginMain(audioMasterCallback audioMaster);

    AEffect * main_plugin (audioMasterCallback audioMaster) asm ("main");

#define main main_plugin

    VST_EXPORT AEffect * main(audioMasterCallback audioMaster)
    {
        return VSTPluginMain(audioMaster);
    }
}


VstIntPtr dispatcher(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
	RemotePluginClient *plugin = (RemotePluginClient *) effect->user;

	VstIntPtr v = 0;

struct ERect retRect = {0, 0, 10, 10};



	switch (opcode)
	{
		case effEditGetRect:
                *((struct ERect **)ptr) = &retRect;
			break;
		case effEditIdle:
			plugin->effVoidOp(effEditIdle);
			break;
		case effStartProcess:
			plugin->effVoidOp(effStartProcess);
			break;
		case effStopProcess:
			plugin->effVoidOp(effStopProcess);
			v = 1;
			break;
		case effGetVendorString:
			strncpy((char *) ptr, plugin->getMaker().c_str(), kVstMaxVendorStrLen);
			((char *) ptr)[kVstMaxVendorStrLen-1] = 0;
			v = 1;
			break;
		case effGetEffectName:
			strncpy((char *) ptr, plugin->getName().c_str(), kVstMaxEffectNameLen);
			((char *) ptr)[kVstMaxEffectNameLen-1] = 0;
			v = 1;
			break;
		case effGetParamName:
			strncpy((char *) ptr, plugin->getParameterName(index).c_str(), kVstMaxParamStrLen);
			((char *) ptr)[kVstMaxParamStrLen] = 0;
			v = 1;
			break;
		case effGetParamLabel:
			plugin->getEffString(effGetParamLabel, index, (char *) ptr, kVstMaxParamStrLen);
			v = 1;
			break;
		case effGetParamDisplay:
			plugin->getEffString(effGetParamDisplay, index, (char *) ptr, kVstMaxParamStrLen);
			v = 1;
			break;
		case effGetProgramName:
			strncpy((char *) ptr, plugin->getProgramName(index).c_str(), kVstMaxProgNameLen);
			((char *) ptr)[kVstMaxProgNameLen-1] = 0;
			break;
		case effSetSampleRate:
			plugin->setSampleRate(opt);
			break;
		case effSetBlockSize:
			plugin->setBufferSize ((VstInt32)value);	
			break;
		case effGetVstVersion:
			v = kVstVersion;
			break;
		case effGetPlugCategory:
			v = plugin->getEffInt(effGetPlugCategory);
			break;
		case effSetProgram:
			if (value < plugin->getProgramCount()) plugin->setCurrentProgram ((VstInt32)value);
			break;
		case effEditOpen:
			plugin->showGUI();
                    v = 1;
			break;
		case effEditClose:
                  plugin->hideGUI();
                   v = 1;			
			break;
		case effCanDo:
			v = 1;
			break;
		case effProcessEvents:
		v = plugin->processVstEvents((VstEvents *) ptr);
           		break;
		case effGetChunk:
 			v = plugin->getChunk((void **) ptr, index);
			break;
		case effSetChunk:
 			v = plugin->setChunk(ptr, value, index);
			break;
		case effGetProgram:
			v = plugin->getProgram();
			break;
                case effClose:
                plugin->effVoidOp(effClose);
                delete plugin;
                 break;


		default:
		break;		
	}
	return v;
}

void processDouble(AEffect* effect, double** inputs, double** outputs, VstInt32 sampleFrames)
{
	return;
}

void process(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames)
{

RemotePluginClient *plugin = (RemotePluginClient *) effect->user;

if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
{
	plugin->process(inputs, outputs, sampleFrames);

}

	return;
}

void setParameter(AEffect* effect, VstInt32 index, float parameter)
{

RemotePluginClient *plugin = (RemotePluginClient *) effect->user;

if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
{
	
	plugin->setParameter(index, parameter);

}

	return;
}

float getParameter(AEffect* effect, VstInt32 index)
{

float retval;

RemotePluginClient *plugin = (RemotePluginClient *) effect->user;


retval = -1;

if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
{
	
	retval = plugin->getParameter(index);

}

return retval;

}

void initEffect(AEffect *eff, RemotePluginClient *plugin)
{
	memset(eff, 0x0, sizeof(AEffect));
	eff->magic = kEffectMagic;
	eff->dispatcher = dispatcher;
	eff->setParameter = setParameter;
	eff->getParameter = getParameter;
	eff->numInputs = plugin->getInputCount();
	eff->numOutputs = plugin->getOutputCount();
	eff->numPrograms = plugin->getProgramCount();
	eff->numParams = plugin->getParameterCount();
	eff->flags = plugin->getFlags();
        eff->flags &= ~effFlagsCanDoubleReplacing;
        eff->flags |= effFlagsCanReplacing;
	eff->resvd1 = 0;
	eff->resvd2 = 0;
	eff->initialDelay = plugin->getinitialDelay();
	eff->object = (void *) 0x123;
	eff->user = plugin;
	eff->uniqueID = 0xabcd1234;
	eff->version = 100;
	eff->processReplacing = process;
	eff->processDoubleReplacing = processDouble;
}


VST_EXPORT AEffect* VSTPluginMain (audioMasterCallback audioMaster)
{
 	if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
		return 0;  

	RemotePluginClient *plugin;
	try {
		plugin = new RemoteVSTClient(audioMaster);

              usleep(500000);

	    initEffect(plugin->theEffect, plugin);

	} catch (std::string e) {
		return 0;
	}


	return plugin->theEffect;
}

