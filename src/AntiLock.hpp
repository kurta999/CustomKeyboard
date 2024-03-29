#pragma once

#include "utils/CSingleton.hpp"

class AntiLock : public CSingleton < AntiLock >
{
    friend class CSingleton < AntiLock >;

public:
    AntiLock() = default;
    ~AntiLock() = default;

    // !\brief Load exclusions to vector from string
    // \param input [in] Input string to parse
    void LoadExclusions(const std::string& input);

    // !\brief Generate writable string from exclusions
    const std::string SaveExclusions() const;

    // !\brief Process for antilock
    void Process();

    // !\brief Return last activity time
    uint32_t GetLastActivityTime() const { return m_LastActivityTime;  }
    
    // !\brief Is enabled?
    bool is_enabled = false;

    // !\brief Is enabled?
    bool is_suspended = false;

    // !\brief Antilock timeout [s]
    uint32_t timeout = 300;

    // !\brief Is screen saver enabled
    bool is_screensaver = false;

    // !\brief Excluded window list
    std::vector<std::string> exclusions;
private:
    // !\brief Check if current session active (user is logged in)
    // \return Is session active?
    bool IsSessionActive() const;

    // !\brief Simulate user activity
    void SimulateUserActivity();
    
    // !\brief Simulate user activity
    void StartScreenSaver() const;

    // !\brief Return true if the given window is in exclusion list
    // \param p [in] Window name to check
    bool IsAnExclusion(const std::string& p) const;

    // !\brief Mouse step
    bool m_StepForward = false;

    // !\brief Last mouse step
    POINT m_LastStep = { 0, 0 };

    // !\brief Ticks passed since last activity
    uint32_t m_LastActivityTime = 0;

    // !\brief Last execution time
    uint32_t m_LastExecution = 0;
};