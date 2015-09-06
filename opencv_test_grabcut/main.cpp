//
//  main.cpp
//  opencv_test_grabcut
//
//  Created by vk on 15/9/3.
//  Copyright (c) 2015年 quxiu8. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include "grabcut.hpp"
/**
 *  grabcut利用空间换时间，每次都存储计算的结果mask，
 *  在oc调用redo和undo时返回或前进mask vector array
 *
 *  @param argc
 *  @param argv
 *
 *  @return
 */
int main (int argc, char * const argv[]) {
    
    // assumes a continous video sequence in the form of jpegs
    // assumes a certain directory path from root
    
    std::string root = "/Users/lixu/licheng/xcode/lixu_test/";
    std::string basename = "test";
    std::string img_prefix = root + "img/" + basename + "/";
    std::string msk_prefix = root + "mask/"+ basename + "/";
    
    std::stringstream ss;
    
    ss.str("");
    ss << "mkdir -p " + root + "/mask/" + basename;
    system(ss.str().c_str());
    
    GrabCut gb;
    
    vector<int> q(2,100);
    q[0]=1;
    
    int f = 100;		// starting frame of the video
   
    cv::Mat img = cv::imread("/Users/vk/Pictures/SkinColorImg/texture/1.jpg");
    cv::Mat mask;
    gb.run(img, mask);
    //cv::waitKey(0);
    return 0;
}