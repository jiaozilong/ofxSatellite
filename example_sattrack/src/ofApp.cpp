// =============================================================================
//
// Copyright (c) 2010-2014 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#include "ofApp.h"


void ofApp::setup()
{
    ofEnableDepthTest();
	ofEnableAlphaBlending();

    rot = 0;

    scaler = 300 / ofx::Geo::GeoUtils::EARTH_RADIUS_KM;

    colorMap.loadImage("color_map_1024.jpg");

    earthSphere.set(ofx::Geo::GeoUtils::EARTH_RADIUS_KM, 24);
    ofQuaternion quat;
    quat.makeRotate(180, 0, 1, 0);
    earthSphere.rotate(quat);
    earthSphere.mapTexCoords(0,
                             colorMap.getTextureReference().getTextureData().tex_u,
                             colorMap.getTextureReference().getTextureData().tex_t,
                             0);

    myLocation = ofx::Geo::ElevatedCoordinate(51.507406923983446,
                                              -0.12773752212524414,
                                              0.05);

    ofHttpResponse response = ofLoadURL("http://www.celestrak.com/NORAD/elements/resource.txt");

    if (200 == response.status)
    {
        satellites = ofx::Satellite::Utils::loadTLEFromBuffer(response.data);
    }
    else
    {
        ofLogError("ofApp::setup()") << "Unable to load : " << response.error;
    }


    cam.setPosition(0, 0, 0);

}


void ofApp::draw()
{
	ofBackground(0);

    cam.begin();

	ofPushMatrix();
    //rot += 1;
    //ofRotate(rot, 0, 1, 0);
    ofScale(scaler, scaler, scaler);

    ofSetColor(255);
    colorMap.bind();
    earthSphere.draw();
    colorMap.unbind();

    ofQuaternion latRot;
    ofQuaternion longRot;

    std::vector<ofx::Satellite::Satellite>::const_iterator iter = satellites.begin();

    ofx::Geo::ElevatedCoordinate pos;

    Poco::DateTime now;

    while (iter != satellites.end())
    {
        // Only show two satellites to keep things simple.
        if (iter->Name().compare("AQUA") != 0 && iter->Name().compare("TERRA") != 0)
        {
            ++iter;
            continue;
        }

        try
        {
            pos = ofx::Satellite::Utils::toElevatedCoordinate((*iter).find(now).ToGeodetic());
        }
        catch (...)
        {
            // If there is an exception, skip it.
            ++iter;
            continue;
        }

		ofQuaternion latRot;
        ofQuaternion longRot;

		latRot.makeRotate(pos.getLatitude(), 1, 0, 0);
		longRot.makeRotate(pos.getLongitude(), 0, 1, 0);

		//our starting point is 0,0, on the surface of our sphere, this is where the meridian and equator meet
		ofVec3f center = ofVec3f(0,0, pos.getElevation() / 1000 + ofx::Geo::GeoUtils::EARTH_RADIUS_KM);
		//multiplying a quat with another quat combines their rotations into one quat
		//multiplying a quat to a vector applies the quat's rotation to that vector
		//so to to generate our point on the sphere, multiply all of our quaternions together then multiple the centery by the combined rotation
		ofVec3f worldPoint = latRot * longRot * center;

        ofNoFill();
        ofSetColor(255, 0, 0, 255);
        ofDrawSphere(worldPoint, 30);

        ofSetColor(255);

		//set the bitmap text mode billboard so the points show up correctly in 3d
        std::stringstream ss;

        ss << iter->Name() << std::endl;
        ss << " Latitude (deg): " << pos.getLatitude() << std::endl;
        ss << "Longitude (deg): " << pos.getLongitude() << std::endl;
        ss << " Elevation (km): " << pos.getElevation() / 1000 << std::endl;

        ofDrawBitmapString(ss.str(), worldPoint);

        ++iter;
    }

	ofPopMatrix();

    cam.end();

}
