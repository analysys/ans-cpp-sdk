ifeq ($(shell uname), Linux)
path += -L ./lib/linux/ 
else 
path += -I ./lib/macos/ 
endif

all: GXX clean
GXX:
	g++ Json.cpp  AnalysysCPlusPlusSdk.cpp AnalysysCPlusSdkDemo.cpp HttpSender.cpp -I ./lib/include $(path)  -D CURL_STATICLIB -L ./ -lcurl -lz -lpthread -std=c++11 -o Demo

clean:

# else
# all: GXX clean
# GXX:
#     g++ Json.cpp  AnalysysCPlusPlusSdk.cpp AnalysysCPlusSdkDemo.cpp HttpSender.cpp -I ./lib/include -L ./lib/linux/ -D CURL_STATICLIB -L ./ -lcurl -lz -lpthread -std=c++11 -o Demo
# clean:

# endif 

	
