# Building LevelDB On Windows

## Prereqs 

Install the [Windows Software Development Kit version 7.1](http:

Download and extract the [Snappy source distribution](http:

1. Open the "Windows SDK 7.1 Command Prompt" :
   Start Menu -> "Microsoft Windows SDK v7.1" > "Windows SDK 7.1 Command Prompt"
2. Change the directory to the leveldb project

## Building the Static lib 

* 32 bit Version 

        setenv /x86
        msbuild.exe /p:Configuration=Release /p:Platform=Win32 /p:Snappy=..\snappy-1.0.5

* 64 bit Version 

        setenv /x64
        msbuild.exe /p:Configuration=Release /p:Platform=x64 /p:Snappy=..\snappy-1.0.5


## Building and Running the Benchmark app

* 32 bit Version 

	    setenv /x86
	    msbuild.exe /p:Configuration=Benchmark /p:Platform=Win32 /p:Snappy=..\snappy-1.0.5
		Benchmark\leveldb.exe

* 64 bit Version 

	    setenv /x64
	    msbuild.exe /p:Configuration=Benchmark /p:Platform=x64 /p:Snappy=..\snappy-1.0.5
	    x64\Benchmark\leveldb.exe

