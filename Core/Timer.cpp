#include "Timer.h"
#include <thread>

namespace X
{
    CTimer::CTimer()
    {
        reset();
    }

    void CTimer::pause(void)
    {
        _mbPaused = true;
    }

    void CTimer::resume(void)
    {
        _mbPaused = false;
    }

    float CTimer::getSecondsPast(void) const
    {
        if (_mbPaused)
            return 0.0f;
        return static_cast<float>(_mdDeltaSec);
    }

    void CTimer::update(void)
    {
        _mdTimePointNew = std::chrono::steady_clock::now();
        _mdTimeDeltaSec = _mdTimePointNew - _mdTimePointOld;
        _mdTimePointOld = _mdTimePointNew;// std::chrono::steady_clock::now();

        _mdDeltaSec = _mdTimeDeltaSec.count();

        // Compute FPS
        ++_muiNumFrames;
        _mdFPSFrameTime += _mdDeltaSec * 1000.0;
        if (_mdFPSFrameTime > 0.0)
        {
            _mdFPS = _muiNumFrames * (1000.0 / _mdFPSFrameTime);

            _muiNumFrames = 0;
            _mdFPSFrameTime = 0.0;
        }

        // Compute FPS smoothed
        _mdFPSAveragedTimeCount += _mdDeltaSec;
        ++_miFPSAveragedNumCallsPerSec;
        _mdFPSAveragedAccum += _mdFPS;
        if (_mdFPSAveragedTimeCount > _mdFPSAveragedRate)
        {
            if (_miFPSAveragedNumCallsPerSec < 1)
                _miFPSAveragedNumCallsPerSec = 1;
            _mdFPSAveraged = _mdFPSAveragedAccum / _miFPSAveragedNumCallsPerSec;
            _mdFPSAveragedTimeCount = 0;
            _miFPSAveragedNumCallsPerSec = 0;
            _mdFPSAveragedAccum = 0;
        }

        // Runtime
        _mdRuntimeInSeconds += _mdDeltaSec;

        // Minimum and maximum acheived framerate
        _mdFPSMinMaxDelay -= _mdDeltaSec;
        if (_mdFPSMinMaxDelay <= 0.0)
        {
            if (_mbFPSMinMaxInit)
            {
                _mbFPSMinMaxInit = false;
                _mdFPSMinimum = 999999;
                _mdFPSMaximum = 0;
            }
            if (!_mbFPSMinMaxInit)
            {
                _mdFPSMinMaxDelay = 0.0;
                if (_mdFPSMinimum > _mdFPS)
                    _mdFPSMinimum = _mdFPS;
                if (_mdFPSMaximum < _mdFPS)
                    _mdFPSMaximum = _mdFPS;
            }
        }


    }

    void CTimer::sleep(unsigned int uiMilliseconds) const
    {
        //Sleep(uiMilliseconds); // Legacy Windows function. Use modern crossplatform method instead...
        std::this_thread::sleep_for(std::chrono::milliseconds(uiMilliseconds));
    }

    void CTimer::setAveragedFPSRate(float fSecondsBetweenUpdates)
    {
        _mdFPSAveragedRate = (double)fSecondsBetweenUpdates;
        if (_mdFPSAveragedRate <= 0)
            _mdFPSAveragedRate = 0.001;
    }

    float CTimer::getAveragedFPSRate(void) const
    {
        return static_cast<float>(_mdFPSAveragedRate);
    }

    float CTimer::getFPS(void) const
    {
        return static_cast<float>(_mdFPS);
    }

    float CTimer::getFPSAveraged(void) const
    {
        return static_cast<float>(_mdFPSAveraged);
    }

    float CTimer::getFPSAveragedTimeUntilNextUpdate(void) const
    {
        return float(_mdFPSAveragedRate - _mdFPSAveragedTimeCount);
    }

    float CTimer::getFPSMinimum(void) const
    {
        return float(_mdFPSMinimum);
    }

    float CTimer::getFPSMaximum(void) const
    {
        return float(_mdFPSMaximum);
    }

    void CTimer::reset(void)
    {
        _mbPaused = false;
        _mdTimePointNew = std::chrono::steady_clock::now();
        _mdTimePointOld = _mdTimePointNew;// std::chrono::steady_clock::now();
        _mdTimeDeltaSec = _mdTimePointNew - _mdTimePointOld;
        _mdDeltaSec = _mdTimeDeltaSec.count();

        // Stuff for FPS
        _mdFPS = 1.0;                // Holds computed current frames per second value
        _muiNumFrames = 0;           // Used to compute FPS stuff        
        _mdFPSFrameTime = 0;         // Used to compute FPS stuff

        // Stuff for FPS averaged
        _mdFPSAveraged = 1.0;                    // Holds computed averaged frames per second value
        _mdFPSAveragedRate = 3.0;                // Number of seconds between updating the value returned by getFPSAveraged() method.
        _mdFPSAveragedTimeCount = 0;             // Used to compute FPSAveraged stuff
        _miFPSAveragedNumCallsPerSec = 0;        // Used to compute FPSAveraged stuff
        _mdFPSAveragedAccum = 0;                 // Used to compute FPSAveraged stuff

        _mdRuntimeInSeconds = 0;

        _mdFPSMinimum = 0;
        _mdFPSMaximum = 0;
        _mdFPSMinMaxDelay = 1.0;
        _mbFPSMinMaxInit = true;
    }

    float CTimer::getRuntimeSeconds(void) const
    {
        return (float)_mdRuntimeInSeconds;
    }

    void CTimer::getClock(float& fSeconds, int& iMinutes, int& iHours, int& iDays, int& iWeeks) const
    {
        fSeconds = 0.0f;
        iMinutes = 0;
        iHours = 0;
        iDays = 0;
        iWeeks = 0;

        // How many seconds are in a week?
        // 60seconds in a minute, 60 minutes in an hour, 24 hours in a day and 7 days in a week.
        // 60 * 60 = 3600 seconds per hour
        // 3600 * 24 = 86400 seconds in a day
        // 86400 * 7 = 604800 seconds in a week
        double seconds = _mdRuntimeInSeconds;
        while (seconds >= 604800)   // Weeks
        {
            seconds -= 604800;
            iWeeks++;
        }
        while (seconds >= 86400)    // Days
        {
            seconds -= 86400;
            iDays++;
        }
        while (seconds >= 3600)     // Hours
        {
            seconds -= 3600;
            iHours++;
        }
        while (seconds >= 60)       // Minutes
        {
            seconds -= 60;
            iMinutes++;
        }
        fSeconds = (float)seconds;  // Seconds
    }

    std::string CTimer::getClock(void) const
    {
        float fSecs = 0.0f;
        int iMins, iHours, iDays, iWeeks = 0;
        getClock(fSecs, iMins, iHours, iDays, iWeeks);
        std::string strRuntime = std::to_string((int)fSecs) + "sec ";
        strRuntime += std::to_string(iMins) + "min ";
        strRuntime += std::to_string(iHours) + "hr ";
        strRuntime += std::to_string(iDays) + "days ";
        strRuntime += std::to_string(iWeeks) + "weeks.";
        return strRuntime;
    }
}