#pragma once

#include "utils/CSingleton.hpp"

class AntiLock : public CSingleton < AntiLock >
{
    friend class CSingleton < AntiLock >;

public:
    AntiLock() = default;
    ~AntiLock() = default;

    // \brief Load exclusions to vector from string
    // \param input [in] Input string to parse
    void LoadExclusions(const std::string& input);

    // \brief Generate writable string from exclusions
    const std::string SaveExclusions();

    // \brief Process for antilock
    void Process();

    // \brief Return last activity time
    uint32_t GetLastActivityTime() { return m_LastActivityTime;  }
    
    // \brief Is enabled?
    bool is_enabled = false;

    // \brief Antilock timeout [s]
    uint32_t timeout = 300;

    // \brief Is screen saver enabled
    bool is_screensaver = false;

    // \brief Excluded window list
    std::vector<std::string> exclusions;
private:
    // \brief Check if current session active (user is logged in)
    // \return Is session active?
    bool IsSessionActive();

    // \brief Simulate user activity
    void SimulateUserActivity();    
    
    // \brief Simulate user activity
    void StartScreenSaver();

    // \brief Return true if the given window is in exclusion list
    // \param p [in] Window name to check
    bool IsAnExclusion(std::string&& p);

    // \brief Mouse step
    bool m_StepForward = false;

    // \brief Ticks passed since last activity
    uint32_t m_LastActivityTime = 0;
};