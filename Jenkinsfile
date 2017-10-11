node("build-node")
{
	stage("Build Client")
	{
		echo 'Building Client..'
		
		//if test_detail exists, remove test_detail
		//sh '[ -f ClientProject/ClientProject/test_detail.xml] || rm ClientProject/ClientProject/test_detail.xml'
		
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
		sh './a.out --gtest_output=xml || true'
		//sh 'sudo ./a.out --gtest_output=xml'
		//sh 'sudo -S ./a.out --gtest_output=xml'
		}
		echo 'testing complete'
	}
	stage("Cleanup")
	{
		//archive
		archive 'ClientProject/ClientProject/test_detail.xml'
		
		//test
		junit 'ClientProject/ClientProject/test_detail.xml'
		
		//remove
		sh 'rm ClientProject/ClientProject/test_detail.xml'
		
		//remove xml file
		//remove a.out
	}
}