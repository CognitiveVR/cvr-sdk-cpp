node("build-node")
{
	stage("Build Client")
	{
		echo 'Building Client..'
		
		checkout scm
		cd ClientProject/ClientProject
		
 sudo g++ ../../CognitiveVRAnalytics/CognitiveVRAnalytics/*.c* include/googletest/src/gtest-all.cc ClientProject.cpp stdafx.cpp -std=c++11 -I ../../CognitiveVRAnalytics/CognitiveVRAnalytics -I include/curl -I include/googletest -I include/googletest/include -I include/googletest/internal/custom -lcurl
		
		echo 'gcc build client'
	}
	stage("Test")
	{
		echo 'Testing..'
		./a.out
	}
}