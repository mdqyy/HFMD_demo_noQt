#include "CDataset.h"

CParamset& CParamset::operator+=(const CParamset& obj){
    this->setCenterPoint(this->getCenterPoint() + obj.getCenterPoint());
    double tempAngle[3];
    for(int i = 0; i < 3; ++i)
        tempAngle[i] = this->getAngle()[i] + obj.getAngle()[i];
    this->setAngle(tempAngle);

    return *this;
}

CParamset& CParamset::operator/=(const float& div){
    this->setCenterPoint(cv::Point(this->getCenterPoint().x / (int)div, this->getCenterPoint().y / (int)div));
    double tempAngle[3];
    for(int i = 0; i < 3; ++i)
        tempAngle[i] = this->getAngle()[i] / div;
    this->setAngle(tempAngle);

    return *this;
}

std::string CParamset::outputParam(){
    std::stringstream sstream;

    sstream << //this->className << " " <<
               this->centerPoint.x << " " << this->centerPoint.y << " "<<
               this->angle[0] << " " <<
               this->angle[1] << " " <<
               this->angle[2] << " ";
    return sstream.str();

    //    write<std::string>(out, this->className);
    //    write<cv::Point>(out, this->centerPoint);
    //    write<double>(out, this->angle);
}

//void CParamset::readParam(std::sstream &in){
//    in >> this->className;
//    in >> this->centerPoint;
//    in >> this->angle;
//}

void cropImageAndDepth(cv::Mat* rgb, cv::Mat* depth, double mindist, double maxdist){
    cv::Mat depthForView = cv::Mat(depth->rows, depth->cols, CV_8U);

    cv::Mat allMinDepth = cv::Mat::ones(depth->rows, depth->cols, CV_16U) * (ushort)mindist;
    cv::Mat allMaxDepth = cv::Mat::ones(depth->rows, depth->cols, CV_16U) * (ushort)maxdist;

    //cv::min(*depth, allMaxDepth, *depth);
    //cv::max(*depth, allMinDepth, *depth);

    //    depth->convertTo(depthForView, CV_8U, 255.0 / 1000.0);
    //    cv::namedWindow("test");
    //    cv::imshow("test", *rgbm);
    //    cv::waitKey(0);
    //    cv::imshow("test",depthForView);
    //    cv::waitKey(0);
    //    cv::destroyAllWindows();

    for(int i = 0; i < depth->rows; ++i){
        for(int j = 0; j < depth->cols; ++j){
            if(depth->at<ushort>(i,j) == (ushort)maxdist)
                depth->at<ushort>(i, j) = 0;
            if(depth->at<ushort>(i,j) == 0){
                for(int k = 0; k < 3; ++k)
                    rgb->at<cv::Vec3b>(i,j)[k] = 0;

            }
        }
    }

    *depth -= allMinDepth;

    //    cv::namedWindow("test");
    //    cv::imshow("test", *rgb);
    //    cv::waitKey(0);
    //    cv::destroyAllWindows();
}

CDataset::CDataset()
    :imgFlag(0),
      featureFlag(0)
{
    img.clear();
    feature.clear();
}

CDataset::~CDataset(){
    if(imgFlag){
        releaseImage();
        //std::cout << "image released!" << std::endl;
    }

    if(featureFlag){
        releaseFeatures();
        //std::cout << "feature released!" << std::endl;
    }
}

int CDataset::loadImage(const CConfig &conf){
    cv::Mat *rgbImg, *depthImg;

    if(!conf.demoMode){
        rgbImg = new cv::Mat;
        *rgbImg = cv::imread(rgb,3).clone();
        if(rgbImg->empty()){
            std::cout << "error! rgb image file " << rgb << " not found!" << std::endl;
            exit(-1);
        }
        depthImg = new cv::Mat;
        *depthImg = cv::imread(depth, CV_LOAD_IMAGE_ANYDEPTH).clone();
        if(depthImg->empty()){
            std::cout << "error! depth image file " << depth << " not found!" << std::endl;
            img.push_back(rgbImg);
            imgFlag  = 1;
            return -1;
        }
    }


    if(!conf.demoMode){
        if(conf.learningMode != 2)
            cropImageAndDepth(rgbImg, depthImg, conf.mindist, conf.maxdist);
    }else{
        if(conf.learningMode != 2)
            cropImageAndDepth(img.at(0), img.at(1), conf.mindist, conf.maxdist);
    }



    if(!conf.demoMode){
        img.push_back(rgbImg);
        img.push_back(depthImg);
    }

    imgFlag  = 1;

    return 0;
}

int CDataset::loadImage(const CConfig &conf, const std::string modelName, const CParamset* param){
    cv::Mat *rgbImg, *depthImg;
    //std::cout << rgb << " " << depth << std::endl;
//    CGlObjLoader obj(this->getModelPath().c_str()); //= new CGlObjLoader(this->getModelPath().c_str());
//    cv::vector<cv::Mat *> tempImage = obj.getAppearance(param->getAngle());



//    rgbImg = tempImage.at(0);
//    depthImg = tempImage.at(1);

    //    rgbImg = new cv::Mat(480,640,CV_8U);// = cv::Mat::zeros(480,640,CV_8U);
    //    depthImg = new cv::Mat(480,640,CV_8U);// = cv::Mat::zeros(480,640,CV_8U);

    //    *rgbImg = cv::Mat::ones(480, 640, CV_8U);
    //    *depthImg = cv::Mat::ones(480, 640, CV_8U

    this->img.push_back(rgbImg);
    this->img.push_back(depthImg);

    //std::cout << depthImg->type() << std::endl;

    //delete obj;

    imgFlag  = 1;

    return 0;
}

int CDataset::releaseImage(){
    if(imgFlag == 0){
        std::cout << "image is already released! foolish!" << std::endl;
        return -1;
    }

    for(int i = 0; i < img.size(); ++i){
        delete img.at(i);
    }

    imgFlag = 0;

    return 0;
}

int CParamset::showParam(){
    std::cout << "name : " << this->className;
    std::cout << " center point : " << this->centerPoint << std::endl;
    std::cout << " Pose : roll " << this->angle[0];
    std::cout << " pitch " << this->angle[1];
    std::cout << " yaw " << this->angle[2];


    std::cout << std::endl;
    return 0;
}

int CDataset::extractFeatures(const CConfig& conf){

    int imgRow = this->img.at(0)->rows, imgCol = this->img.at(0)->cols;
    cv::Mat *integralMat;

    if(conf.learningMode != 1){
        if(conf.rgbFeature == 1){ // if got rgb image only, calc hog feature
            feature.clear();
            feature.resize(32);
            for(int i = 0; i < 32; ++i)
                feature.at(i) = new cv::Mat(imgRow, imgCol, CV_8UC1);

            cv::cvtColor(*img.at(0), *(feature.at(0)), CV_RGB2GRAY);

            cv::Mat I_x(imgRow, imgCol, CV_16SC1);
            cv::Mat I_y(imgRow, imgCol, CV_16SC1);


            cv::Sobel(*(feature.at(0)), I_x, CV_16S, 1, 0);
            cv::Sobel(*(feature.at(0)), I_y, CV_16S, 0, 1);

            cv::convertScaleAbs(I_x, *(feature[3]), 0.25);
            cv::convertScaleAbs(I_y, *(feature[4]), 0.25);

            // Orientation of gradients
            for(int  y = 0; y < img.at(0)->rows; y++)
                for(int  x = 0; x < img.at(0)->cols; x++) {
                    // Avoid division by zero
                    float tx = (float)I_x.at<short>(y, x) + (float)copysign(0.000001f, I_x.at<short>(y, x));
                    // Scaling [-pi/2 pi/2] -> [0 80*pi]
                    feature.at(1)->at<uchar>(y, x) = (uchar)(( atan((float)I_y.at<short>(y, x) / tx) + 3.14159265f / 2.0f ) * 80);
                    //std::cout << "scaling" << std::endl;
                    feature.at(2)->at<uchar>(y, x) = (uchar)sqrt((float)I_x.at<short>(y, x)* (float)I_x.at<short>(y, x) + (float)I_y.at<short>(y, x) * (float)I_y.at<short>(y, x));
                }

            // Magunitude of gradients
            for(int y = 0; y < img.at(0)->rows; y++)
                for(int x = 0; x < img.at(0)->cols; x++ ) {
                    feature.at(2)->at<uchar>(y, x) = (uchar)sqrt(I_x.at<short>(y, x)*I_x.at<short>(y, x) + I_y.at<short>(y, x) * I_y.at<short>(y, x));
                }

            hog.extractOBin(feature[1], feature[2], feature, 7);

            // calc I_xx I_yy
            cv::Sobel(*(feature.at(0)), I_x, CV_16S, 2, 0);
            cv::Sobel(*(feature.at(0)), I_y, CV_16S, 0, 2);

            cv::convertScaleAbs(I_x, *(feature[5]), 0.25);
            cv::convertScaleAbs(I_y, *(feature[6]), 0.25);

            cv::Mat img_Lab;
            cv::cvtColor(*img.at(0), img_Lab, CV_RGB2Lab);
            cv::vector<cv::Mat> tempfeature(3);

            cv::split(img_Lab, tempfeature);

            for(int i = 0; i < 3; ++i)
                tempfeature.at(i).copyTo(*(feature.at(i)));

            // min max filter
            for(int c = 0; c < 16; ++c)
                minFilter(feature[c], feature[c + 16], 5);
            for(int c = 0; c < 16; ++c)
                maxFilter(feature[c], feature[c], 5);

        }else{
            feature.clear();

            // calc gray integral image
            cv::Mat grayImg(imgRow + 1, imgCol, CV_8U);
            cv::cvtColor(*img.at(0), grayImg, CV_RGB2GRAY);
            integralMat = new cv::Mat(imgRow + 1, imgCol + 1, CV_64F);
            cv::integral(grayImg, *integralMat, CV_64F);
            feature.push_back(integralMat);

            // calc r g b integral image
            std::vector<cv::Mat> splittedRgb;
            cv::split(*img.at(0), splittedRgb);
            for(int i = 0; i < splittedRgb.size(); ++i){
                integralMat = new cv::Mat(imgRow + 1, imgCol + 1, CV_64F);
                cv::integral(splittedRgb.at(i), *integralMat, CV_64F);
                feature.push_back(integralMat);
            }



            featureFlag = 1;
        }
    }

    if(img.size() > 1){
        cv::Mat tempDepth = cv::Mat(img.at(0)->rows, img.at(0)->cols, CV_8U);// = *img.at(1);

        if(img.at(1)->type() != CV_8U)
            img.at(1)->convertTo(tempDepth, CV_8U, 255.0 / (double)(conf.maxdist - conf.mindist));
        else
            tempDepth = *img.at(1);
        integralMat = new cv::Mat(imgRow + 1, imgCol + 1, CV_64F);
        cv::integral(tempDepth, *integralMat, CV_64F);
        feature.push_back(integralMat);

        featureFlag  = 1;
    }

    return 0;
}

int CDataset::releaseFeatures(){
    if(featureFlag == 0){
        std::cout << "image is already released! foolish!" << std::endl;
        return -1;
    }

    for(int i = 0; i < feature.size(); ++i){
        if(feature.at(i) != NULL){
            delete feature.at(i);
            feature.at(i) = NULL;
        }
    }

    featureFlag = 0;

    return 0;
}

void CDataset::minFilter(cv::Mat* src, cv::Mat* des, int fWind) {
    int d = (fWind - 1) / 2;
    cv::Rect roi;
    cv::Mat desTemp(src->rows, src->cols, CV_8U), vTemp;

    for(int y = 0; y < src->rows - fWind; ++y){ //for image height
        if(y < fWind)
            roi = cv::Rect(0, 0, src->cols, fWind - y);
        else
            roi = cv::Rect(0, y, src->cols, fWind);

        cv::reduce((*src)(roi), vTemp, 0, CV_REDUCE_MIN);

        roi = cv::Rect(0, y + d, src->cols, 1);
        //cv::Mat roiDesTemp(desTemp, roi);
        vTemp.copyTo(desTemp(roi));
    }// For image height

    for(int x = 0; x < src->cols - fWind; ++x){ // for image width
        if(x < d)
            roi = cv::Rect(0, 0, fWind - x, src->rows);
        else
            roi = cv::Rect(x, 0, fWind, src->rows);

        cv::reduce(desTemp(roi), vTemp, 1, CV_REDUCE_MIN);

        roi = cv::Rect(x + d, 0, 1, src->rows);
        cv::Mat roiDesTemp((*des), roi);
        vTemp.copyTo((*des)(roi));// = vTemp.clone();//copyTo((*des)(roi));
    } // for image width
}

void CDataset::maxFilter(cv::Mat* src, cv::Mat* des, int fWind) {
    int d = (fWind - 1) / 2;
    cv::Rect roi;
    cv::Mat desTemp(src->rows, src->cols, CV_8U), vTemp;

    for(int y = 0; y < src->rows - fWind; ++y){ //for image height
        if(y < fWind)
            roi = cv::Rect(0, 0, src->cols, fWind - y);
        else
            roi = cv::Rect(0, y, src->cols, fWind);

        cv::reduce((*src)(roi), vTemp, 0, CV_REDUCE_MAX);

        roi = cv::Rect(0, y + d, src->cols, 1);
        cv::Mat roiDesTemp(desTemp, roi);
        vTemp.copyTo(desTemp(roi));
    }// For image height

    for(int x = 0; x < src->cols - fWind; ++x){ // for image width
        if(x < d)
            roi = cv::Rect(0, 0, fWind - x, src->rows);
        else
            roi = cv::Rect(x, 0, fWind, src->rows);

        cv::reduce(desTemp(roi), vTemp, 1, CV_REDUCE_MAX);

        roi = cv::Rect(x + d, 0, 1, src->rows);
        cv::Mat roiDesTemp(*des, roi);
        vTemp.copyTo((*des)(roi));// = vTemp.clone();//copyTo((*des)(roi));
    } // for image width

}

CNegDataset convertPosToNeg2(CPosDataset &pos)
{
    CNegDataset tempNegDataset;
    tempNegDataset.setRgbImagePath(pos.getRgbImagePath());
    tempNegDataset.setDepthImagePath(pos.getDepthImagePath());
    tempNegDataset.setModelPath(pos.getModelPath());

    return tempNegDataset;
}
