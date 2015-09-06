//
//  grabcut.cpp
//  opencv_test_grabcut
//
//  Created by vk on 15/9/3.
//  Copyright (c) 2015年 quxiu8. All rights reserved.
//


#include "grabcut.hpp"

GrabCut::GrabCut()
{
    _mode=GC_FGD;
}	

void GrabCut::run(Mat img, Mat &msk)
{
    cout << "run grabcut" << endl;
    _src	= img;
    _cutResultMask = Mat(img.size(), CV_8UC1, Scalar(0));
    _maskStore = Mat(img.size(), CV_8UC1, Scalar(0));
    _mask	= Mat::ones(_src.size(),CV_8UC1)*GC_PR_BGD;
    _bin	= Mat::zeros(_src.size(),CV_8UC1);
    cout << "GC_BGD " << GC_BGD <<endl;				// 0
    cout << "GC_FGD " << GC_FGD <<endl;				// 1
    cout << "GC_PR_BGD " << GC_PR_BGD <<endl;		// 2
    cout << "GC_PR_FGD " << GC_PR_FGD <<endl;		// 3
    _name = "graphcut";
    namedWindow(_name);
    setMouseCallback(_name, wevents,this);
    Rect roi(0,0,_src.cols,_src.rows);
    _dsp = Mat::zeros(_src.rows*2,_src.cols*2,CV_8UC3);
    _src.copyTo(_dsp(roi));
    //_dsp(roi) = _src.clone();
    cout << "loop" << endl;
    while(1)
    {
        imshow(_name,_dsp);
        char c = waitKey(1);				// 
        
        if(c=='d')							// done
        {			
            msk = _bin*1.0;					// output
            break;
        }
        else if(c=='f') _mode = GC_FGD;		// forground mode
        else if(c=='b') _mode = GC_BGD;		// background mode
        else if(c=='r')						// reset
        {
            _src.copyTo(_dsp(roi));			// 
            _mask	= GC_PR_BGD;
            _gcut	= GC_PR_BGD;
            show();
        }
    }
    destroyWindow(_name);
}

void GrabCut::show()
{
    Scalar fg_color(255,0,0);
    Scalar bg_color(0,255,0);
    cv::Mat scribbled_src = _src.clone();
    const float alpha = 0.7f;
    
    for(int y=0; y < _gcut.rows; y++){
        for(int x=0; x < _gcut.cols; x++){
            if(_gcut.at<uchar>(y, x) == cv::GC_FGD) {
                cv::circle(scribbled_src, cv::Point(x, y), 2, fg_color, -1);
            } 
            else if(_gcut.at<uchar>(y, x) == cv::GC_BGD) {
                cv::circle(scribbled_src, cv::Point(x, y), 2, bg_color, -1);
            } 
            else if(_gcut.at<uchar>(y, x) == cv::GC_PR_BGD) {
                cv::Vec3b& pix = scribbled_src.at<cv::Vec3b>(y, x);
                pix[0] = (uchar)(pix[0] * alpha + bg_color[0] * (1-alpha));
                pix[1] = (uchar)(pix[1] * alpha + bg_color[1] * (1-alpha));
                pix[2] = (uchar)(pix[2] * alpha + bg_color[2] * (1-alpha));
            } 
            else if(_gcut.at<uchar>(y, x) == cv::GC_PR_FGD) {
                cv::Vec3b& pix = scribbled_src.at<cv::Vec3b>(y, x);
                pix[0] = (uchar)(pix[0] * alpha + fg_color[0] * (1-alpha));
                pix[1] = (uchar)(pix[1] * alpha + fg_color[1] * (1-alpha));
                pix[2] = (uchar)(pix[2] * alpha + fg_color[2] * (1-alpha));
            }
        }
    }
    Rect roi;
    Mat scrb;
    roi = Rect(_src.cols,0,_src.cols,_src.rows);
    scribbled_src.copyTo(_dsp(roi));
    
    Mat msk = getBinMask();
    cv::Mat send = msk.clone();
    cv::Mat sendFilter;
    cv::Mat resultMat = filterMaskAndMergeMat(send, _cutResultMask, _gcutBuffer);
    Mat fg = getFGByMask(resultMat);//getFG();
    roi = Rect(_src.cols,_src.rows,_src.cols,_src.rows);
    fg.copyTo(_dsp(roi));
  //  imshow("fg", fg);
    
   // imshow("mergeResultMat", resultMat);
    _cutResultMask = resultMat;
    //Rect    tmp = getMaskRct(_gcutBuffer);
    cvtColor(msk,msk,COLOR_GRAY2BGR);
    roi = Rect(0,_src.rows,_src.cols,_src.rows);
    msk.copyTo(_dsp(roi));
    //imshow("msk", msk);
    //imshow("_mask", _mask);
    imshow(_name,_dsp);
    waitKey(1);
}

void GrabCut::showBit(cv::Mat gray, cv::Mat &out)
{
    if(gray.channels() == 1)
    {
        int rows = gray.rows;
        int cols = gray.cols;
        int maxData = 0;
        cv::Mat showMat = cv::Mat(gray.size(),CV_8UC3,cv::Scalar(0,0,0));
        
        for(int y = 0;y<rows;y++){
            uchar *maskRowData = gray.ptr<uchar>(y);
            for (int x = 0; x<cols; x++) {
                if(maskRowData[x]>maxData){
                    maxData = maskRowData[0];
                }
            }
        }
        printf("maxData = %d\n",maxData);
        
        for(int y = 0;y<rows;y++){
            uchar *maskRowData = gray.ptr<uchar>(y);
            uchar *showMatRowData = showMat.ptr<uchar>(y);
            for (int x = 0; x<cols; x++) {
                if(maskRowData[x] == 1){
                    showMatRowData[x*3 + 0] = 255;
                    showMatRowData[x*3 + 1] = 0;
                    showMatRowData[x*3 + 2] = 0;
                }
                if(maskRowData[x] == 2){
                    showMatRowData[x*3 + 0] = 0;
                    showMatRowData[x*3 + 1] = 255;
                    showMatRowData[x*3 + 2] = 255;
                }
            }
        }
        showMat.copyTo(out);
    }
    else
        printf("Channels is not 1\n");
}

/**
 *  得到当前mask的画线rect
 *
 *  @param maskMat <#maskMat description#>
 *
 *  @return <#return value description#>
 */
Rect    GrabCut::getMaskRct( Mat maskMat )
{
    int rows = maskMat.rows;
    int cols = maskMat.cols;
    uchar grayWillBeFind = 1;
    
    Mat tmpMat;
    int lx = cols;
    int rx = 0;
    int ty = rows;
    int by = 0;
    
    for(int y = 0; y<rows; y++ ){
        uchar *maskMatRowsData = maskMat.ptr<uchar>(y);
        for(int x = 0; x<cols; x++ ){
            if(maskMatRowsData[x] == grayWillBeFind )
            {
                if(x<lx){
                    lx = x;
                }
                if(x>rx){
                    rx = x;
                }
                if(y<ty){
                    ty = y;
                }
                if(y>by){
                    by = y;
                }
            }
        }
    }
    
    //扩大一下截图范围
    /*
     if(lx - 10 >= 0)
     lx = lx - 10;
     if(rx + 10 <= cols - 1)
     rx = rx + 10;
     if(ty - 10 >= 0)
     ty = ty - 10;
     if(by + 10 <= rows - 1)
     by = by + 10;
     */
    //cv::Point lt = cv::Point(lx,ty);
    //cv::Point rb = cv::Point(rx,by);
    
    showBit(maskMat, tmpMat );
    Rect rectFind = Rect(lx,ty, rx-lx+1, by-ty+1);
    rectangle(tmpMat, rectFind, Scalar(0,0,255));
    imshow("tmpMat", tmpMat);
    
    return rectFind;
}

/**
 *  将第一个mat中的mergeRect位置上的内容合并的到第二个mat的mergeRect位置上
 *  输入输出都是单通道图像，前景为255背景为0
 *
 *  @param mergeMat   要合并的Mat
 *  @param mergeToMat 要合并到的Mat
 *  @param mergeRect  要合并的区域
 *
 *  @return 合并结果
 */
Mat  GrabCut::mergeMat( Mat mergeMat, Mat mergeToMat, Rect mergeRect )
{
    int liteRows = mergeRect.height;
    int liteCols = mergeRect.width;
    int rows = mergeToMat.rows;
    int cols = mergeToMat.cols;
    
    Mat  newMat = mergeToMat.clone();
    imshow("newMat", newMat);
    
    //Mat liteMat = Mat(liteRows, liteCols, CV_8UC1, Scalar(0));
    for (int y = 0; y<liteRows; y++) {
        uchar *mergeMatRowData = mergeMat.ptr<uchar>(mergeRect.y + y);
        uchar *newMatRowData = newMat.ptr<uchar>(mergeRect.y + y);
        //  uchar *liteMatRowData = liteMat.ptr<uchar>(y);
        for (int x = 0; x<liteCols; x++) {
            newMatRowData[mergeRect.x + x] = mergeMatRowData[mergeRect.x + x] | newMatRowData[mergeRect.x + x];
            //     liteMatRowData[x] = mergeMatRowData[mergeRect.x + x];
        }
    }
    //imshow("liteMat", liteMat);
    
    //liteMat.copyTo(newMat(mergeRect));  //如果用这个则会造成若新的mask与旧mask部分重合且有部分区域少于旧mask则会造成删除旧mask
    return newMat;
}

/**
 *  过滤得到的mask并合并到最终的上次计算的mat中，每次只是增加mat。若本次只找到一个外框则直接进行merge
 *
 *  @param srcMat        本次输入的maskMat
 *  @param matStore      上次的mask计算结果
 *  @param filterRuleMat 过滤条件，输入应该是划屏位置，按照划屏的位置进行选择
 *
 *  @return 返回本次融合的结果
 */
Mat GrabCut::filterMaskAndMergeMat(Mat srcMat, Mat matStore,Mat filterRuleMat)
{
    Mat resultMat = Mat(srcMat.size(),CV_8UC1,Scalar(0));
    
    Mat aMatStore = matStore.clone();
    imshow("aMatStore", aMatStore);
    if (srcMat.channels() != 1){
        printf("EEERRRR\n");
        return resultMat;
    }
    int rows = srcMat.rows;
    int cols = srcMat.cols;
    
    Rect filterRect = getMaskRct(filterRuleMat);
    
    cv::Mat contoursMat = cv::Mat(srcMat.size(), CV_8UC1, Scalar(0));
    vector<vector<Point>> contours;
    findContours( srcMat.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE );
    
    int lx = rows;
    int rx = 0;
    int ty = cols;
    int by = 0;
    /**
     *  计算每个contour的rect,同时与画线的rect比较
     *  若符合要求则留下，否则剔除
     */
    for(int i = 0;i<contours.size();i++){
        lx = rows;
        rx = 0;
        ty = cols;
        by = 0;
        for(int j = 0;j<contours[i].size();j++)
        {
            Point perPoint = contours[i][j];
            if(perPoint.x < lx)
                lx = perPoint.x;
            if(perPoint.x > rx)
                rx = perPoint.x;
            if(perPoint.y < ty)
                ty = perPoint.y;
            if(perPoint.y > by)
                by = perPoint.y;
        }
        Point lt = Point(lx,ty);
        Rect rectDraw = Rect(lx,ty,rx-lx+1,by-ty+1);
        
        if(contours.size() == 1){
            //drawContours( aMatStore, contours, i, Scalar(255), CV_FILLED);
            resultMat = mergeMat( srcMat, aMatStore, rectDraw ); //若只有一个contur则直接返回
        }
        else{
            if( diffRect(rectDraw,filterRect) )
            {
                /**
                 *  画一个contour然后直接当srcMat传入merge函数
                 */
                drawContours( contoursMat, contours, i, Scalar(255), CV_FILLED);
                resultMat = mergeMat(contoursMat, aMatStore, rectDraw);
            }
        }
    }
    printf("filter end\n");
    return resultMat;
}
/**
 *  比较两个rect是否是包含关系，r1包含r2则返回ture,否则是false
 *
 *  @param r1 是否包含r2
 *  @param r2 是否被r1包含
 *
 *  @return r1包含r2则返回ture 否则false
 */
bool   GrabCut::diffRect(Rect r1, Rect r2)
{
    if( r1.x <= r2.x && r1.y <= r2.y ){
        if(r1.width >= r2.width && r1.height >= r2.height){
            if(r1.width + r1.x - 1 >= r2.width + r2.x - 1 && r1.height + r1.y - 1 >= r2.height + r2.y - 1){
                return true;
            }
            else{
                return false;
            }
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
    return false;
}

Mat GrabCut::getFG()
{
    Mat fg = cv::Mat::zeros(_src.size(), _src.type());
    Mat mask = getBinMask();
    _src.copyTo(fg, mask);
    return fg;
}

Mat	GrabCut::getFGByMask(Mat mask)
{
    Mat fg = cv::Mat::zeros(_src.size(), _src.type());
    //Mat aMask = mask;
    _src.copyTo(fg, mask);
    return fg;
    
}

Mat GrabCut::getBinMask()
{
    Mat binmask(_gcut.size(), CV_8U);
    binmask = _gcut & GC_FGD;
    binmask = binmask * 255;
    Mat tmp;
    binmask.copyTo(tmp);
    vector<vector<Point> > co;
    vector<Vec4i> hi;
    binmask *= 0;
    findContours(tmp,co,hi,RETR_EXTERNAL,CHAIN_APPROX_NONE);
    for(int i=0;i<co.size();i++){
        if(contourArea(Mat(co[i])) < 50) continue;
        drawContours(binmask, co,i, CV_RGB(255,255,255), CV_FILLED, CV_AA);
    }
    binmask.copyTo(_bin);
    return binmask;
}

void GrabCut::events( int e, int x, int y, int flags)
{
    printf("events\n");
    _pt = Point(x,y);
    int c;
    switch(e) {
        case EVENT_LBUTTONDOWN:
            ldrag = true;
            lstart = _pt;
            break;
        case EVENT_LBUTTONUP:
            ldrag = false;
            _tmp = _mask & GC_FGD;
            c = countNonZero(_tmp);
            if(c>0)
            {
                _mask.copyTo(_gcut);
                _mask.copyTo(_gcutBuffer);
                Mat showGcut;
                Mat showBgd;
                Mat showFgd;
                showBit(_gcut, showGcut);
                cv::grabCut(_src,_gcut,Rect(), _bgd, _fgd, 1, cv::GC_INIT_WITH_MASK);
                cv::Mat resultMaskMat;
                cv::Mat resultColorMat;
                grabcutByMergeToMatAndMskMat(_maskStore ,_gcut, resultMaskMat, resultColorMat);
                _maskStore = resultMaskMat;
                
                /**
                 *  应该按照指定的maskRect进行计算，但结果还是全局计算
                 */
                //cv::grabCut( _src, _gcut, maskRect, _bgd, _fgd, 1, cv::GC_INIT_WITH_MASK);
                show();
                _mask	= GC_PR_BGD;
            }
            break;
        case EVENT_MOUSEMOVE:
            
            if(ldrag) 
            {
                line(_mask,lstart, _pt, Scalar(_mode), 10);
                
                if(_mode==GC_FGD)		line(_dsp,lstart, _pt,CV_RGB(255,0,0), 10); //CG_FGD = 1
                else if(_mode==GC_BGD)	line(_dsp,lstart, _pt,CV_RGB(0,255,0), 10);
                
                lstart = _pt;
                //cout << _pt << endl;
            }
            break;
        default:
            break;
    };
    //cout << "eventout" << endl;
}

void GrabCut::grabcutByMergeToMatAndMskMat(const Mat mergeToMat ,const Mat msk, Mat & resultMaskMat, Mat &resultColorMat)
{
    cv::Mat srcImage = _src;
    cv::Mat aGcut = msk.clone();
    cv::Mat mskClone = msk.clone();
    cv::Mat aBgd;
    cv::Mat aFgd;
    cv::grabCut(srcImage, aGcut,Rect(), aBgd, aFgd, 1, cv::GC_INIT_WITH_MASK);
    cv::Mat bitMask = getBinMaskByMask(aGcut);
    cv::Mat send = bitMask.clone();
    cv::Mat sendFilter;
    cv::Mat resultMat = filterMaskAndMergeMat(send, mergeToMat, mskClone);
    resultMaskMat = resultMat.clone();
    Mat fg = getFGByMask(resultMat);//getFG();
    imshow("allfg", fg);
}
Mat GrabCut::getBinMaskByMask(cv::Mat mask)
{
    Mat binmask(mask.size(), CV_8U);
    binmask = mask& GC_FGD;
    binmask = binmask * 255;
    Mat tmp;
    binmask.copyTo(tmp);
    vector<vector<Point> > co;
    vector<Vec4i> hi;
    binmask *= 0;
    findContours(tmp,co,hi,RETR_EXTERNAL,CHAIN_APPROX_NONE);
    for(int i=0;i<co.size();i++){
        if(contourArea(Mat(co[i])) < 50) continue;
        drawContours(binmask, co,i, CV_RGB(255,255,255), CV_FILLED, CV_AA);
    }
    binmask.copyTo(_bin);
    return binmask;
}
/**
 *  封装grabCut算法
 *
 *  @param maskPoint
 *  @param lineWidth
 *  @param resultMaskMat
 *  @param resultColorMat 
 */
void GrabCut::processGrabCut(std::vector<cv::Point> maskPoint, int lineWidth, Mat &resultMaskMat, Mat &resultColorMat)
{

}

static void wevents( int e, int x, int y, int flags, void* ptr )
{
    GrabCut *mptr =  (GrabCut*)ptr;
    if(mptr != NULL) mptr->events(e,x,y,flags);
}