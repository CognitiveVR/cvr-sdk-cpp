node("build-node")
{
	stage("Build DLL") {
		echo 'Building DLL..'
		checkout scm
		
		echo 'gcc build dll'
		cd CognitiveVRAnalytics/CognitiveVRAnalytics/
		//g++ -o CognitiveVRAnalytics.so 
		g++ cognitive_log.cc CognitiveVRAnalytics.cpp dllmain.cpp dynamicobject.cpp exitpoll.cpp gazetracker.cc network.cc sensor.cc stdafx.cpp transaction.cc tuning.cc
		echo 'post compile library source'
		
		//g++ *.c* -fPIC -o lib(CognitiveVRAnalytics).so -shared
		
		//https://ubuntuforums.org/showthread.php?t=1173958
		
		mv CognitiveVRAnalytics.so ../../ClientProject/ClientProject/CognitiveVRAnalytics.so
		
		echo 'moved so to clientproject'
	}
	stage("Build Client") {
		echo 'Building Client..'
		
		//when building client, include -l CognitiveVRAnalytics.so to link library?
		//-I /path/to/.so/file -l(CognitiveVRAnalytics) - outputname
		//-I include
		
		echo 'gcc build client'
	}
	stage("Test") {
		echo 'Testing..'
	}
	stage("Deploy") {
		echo 'Deploying....'
	}
}