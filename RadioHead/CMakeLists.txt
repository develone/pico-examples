add_library(radiohead INTERFACE)
target_include_directories(radiohead INTERFACE ${CMAKE_CURRENT_LIST_DIR}/radiohead-lib)
target_sources(radiohead INTERFACE
    

    ${CMAKE_CURRENT_LIST_DIR}/radiohead-lib/RH_ASK.cpp
    ${CMAKE_CURRENT_LIST_DIR}/radiohead-lib/RH_ASK.h
    ${CMAKE_CURRENT_LIST_DIR}/radiohead-lib/RHCRC.h
    ${CMAKE_CURRENT_LIST_DIR}/radiohead-lib/RHCRC.cpp
    ${CMAKE_CURRENT_LIST_DIR}/radiohead-lib/RHGenericDriver.h
    ${CMAKE_CURRENT_LIST_DIR}/radiohead-lib/RHGenericDriver.cpp    
    

    ${CMAKE_CURRENT_LIST_DIR}/radiohead-lib/RadioHead.h
    
 
	

)
