// interface/IComponent.h
#pragma once

#ifndef interface
#define interface struct
#endif

interface IComponent {
    virtual ~IComponent() = default;
    virtual int getId() const = 0;
};
