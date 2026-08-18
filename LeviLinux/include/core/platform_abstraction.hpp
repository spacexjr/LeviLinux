#ifndef PLATFORM_ABSTRACTION_HPP
#define PLATFORM_ABSTRACTION_HPP

#include <string>

class PlatformAbstraction {
public:
    static PlatformAbstraction& instance();

    bool init();
    void cleanup();

    std::string getGraphicsAPI() const;
    std::string getAudioAPI() const;
    std::string getInputAPI() const;

    PlatformAbstraction();
    ~PlatformAbstraction();

private:
    PlatformAbstraction(const PlatformAbstraction&) = delete;
    PlatformAbstraction& operator=(const PlatformAbstraction&) = delete;

    void detectPlatform() const;
    void setupGraphicsShim() const;
    void setupAudioShim() const;
    void setupInputShim() const;
    void setupVulkanShim() const;
    void setupOpenGLESShim() const;
    void setupALSAShim() const;
    void setupPulseAudioShim() const;
    void setupPipeWireShim() const;

    mutable bool initialized = false;
    mutable std::string detectedGraphicsAPI;
    mutable std::string detectedAudioAPI;
    mutable std::string detectedInputAPI;
};

#endif // PLATFORM_ABSTRACTION_HPP

