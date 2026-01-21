include("/home/empr3ss/Desktop/Study/twims/kg/lab2/build/Qt_6_9_1_PATH-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/lab2-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE /home/empr3ss/Desktop/Study/twims/kg/lab2/build/Qt_6_9_1_PATH-Debug/lab2
    GENERATE_QT_CONF
)
