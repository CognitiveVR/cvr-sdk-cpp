node("build-node")
{
	stage("Build Client")
	{
		echo 'Building Client..'
		
		checkout scm
		dir ClientProject/ClientProject
		
		sh 'g++ ../../CognitiveVRAnalytics/CognitiveVRAnalytics/*.c* include/googletest/src/gtest-all.cc ClientProject.cpp stdafx.cpp -std=c++11 -I ../../CognitiveVRAnalytics/CognitiveVRAnalytics -I include/curl -I include/googletest -I include/googletest/include -I include/googletest/internal/custom -lcurl'
		
		echo 'gcc build client'
	}
	stage("Test")
	{
		echo 'Testing..'
		sh './a.out'
	}
}