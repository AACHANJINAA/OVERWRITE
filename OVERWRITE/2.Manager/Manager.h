#pragma once

template <typename T>
class MANAGER
{
public:
    static T& get_instance()
    {
        static T _instance;
        return _instance;
    }

    MANAGER(const MANAGER&) = delete;
    MANAGER& operator=(const MANAGER&) = delete;

protected:
    MANAGER() = default;
    ~MANAGER() = default;
};
