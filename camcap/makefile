cvpath = -IF:/mingcv22/include -LF:/mingcv22/lib
cvlibs = -lopencv_core220 -lopencv_imgproc220 -lopencv_calib3d220 \
         -lopencv_video220 -lopencv_features2d220 -lopencv_ml220 \
         -lopencv_highgui220 -lopencv_objdetect220 -lopencv_contrib220 \
         -lopencv_legacy220

cvlibsd = -lopencv_core220d -lopencv_imgproc220d -lopencv_calib3d220d \
         -lopencv_video220d -lopencv_features2d220d -lopencv_ml220d \
         -lopencv_highgui220d -lopencv_objdetect220d -lopencv_contrib220d \
         -lopencv_legacy220d
gccopts =



Executable : testcap.c
	gcc $(gccopts) -ggdb testcap.c $(cvpath) $(cvlibsd) -o testcap
