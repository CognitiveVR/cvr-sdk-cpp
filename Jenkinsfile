node("build-node")
{
	stage("Build Client")
	{
		echo 'Building Client..'
		
		checkout scm
		dir('ClientProject/ClientProject')
		{
		sh 'g++ ../../CognitiveVRAnalytics/CognitiveVRAnalytics/*.c* include/googletest/src/gtest-all.cc ClientProject.cpp stdafx.cpp -std=c++11 -I ../../CognitiveVRAnalytics/CognitiveVRAnalytics -I include/curl -I include/googletest -I include/googletest/include -I include/googletest/internal/custom -lcurl'
		}
		echo 'gcc build client'
	}
	stage("Test")
	{
		echo 'Testing..'
		dir('ClientProject/ClientProject')
		{
		//sh 'whoami'
		sh './a.out --gtest_output=xml'
		//sh 'sudo ./a.out --gtest_output=xml'
		//sh 'sudo -S ./a.out --gtest_output=xml'
		}
	}
	stage("Cleanup")
	{
		//archive xml file
		//archive 'ClientProject/ClientProject/test_details.xml'
		
		sh 'rm ClientProject/ClientProject/test_details.xml'
		
		//test junit
		//junit 'ClientProject/ClientProject/test_details.xml'
		
		//remove xml file
		//remove a.out
	}
}