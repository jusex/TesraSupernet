





#ifndef CRYPTOPP_OS_SIGNAL_H
#define CRYPTOPP_OS_SIGNAL_H

#include "config.h"

#if defined(UNIX_SIGNALS_AVAILABLE)
# include <signal.h>
#endif

NAMESPACE_BEGIN(CryptoPP)



#if defined(UNIX_SIGNALS_AVAILABLE) || defined(CRYPTOPP_DOXYGEN_PROCESSING)




extern "C" {
    typedef void (*SignalHandlerFn) (int);
};







extern "C" {
	inline void NullSignalHandler(int unused) {CRYPTOPP_UNUSED(unused);}
};















template <int S, bool O=false>
struct SignalHandler
{
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    SignalHandler(SignalHandlerFn pfn = NULL, int flags = 0) : m_installed(false)
    {
        
        struct sigaction new_handler;

        do
        {
            int ret = 0;

            ret = sigaction (S, 0, &m_old);
            if (ret != 0) break; 

            
            if (m_old.sa_handler != 0 && !O) break;

#if defined __CYGWIN__
            
            memset(&new_handler, 0x00, sizeof(new_handler));
#else
            ret = sigemptyset (&new_handler.sa_mask);
            if (ret != 0) break; 
#endif

            new_handler.sa_handler = (pfn ? pfn : &NullSignalHandler);
            new_handler.sa_flags = (pfn ? flags : 0);

            
            ret = sigaction (S, &new_handler, 0);
            if (ret != 0) break; 

            m_installed = true;

        } while(0);
    }

    ~SignalHandler()
    {
        if (m_installed)
            sigaction (S, &m_old, 0);
    }

private:
    struct sigaction m_old;
    bool m_installed;

private:
    
    SignalHandler(const SignalHandler &);
    void operator=(const SignalHandler &);
};
#endif

NAMESPACE_END

#endif 
