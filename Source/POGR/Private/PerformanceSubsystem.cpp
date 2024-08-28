#include "PerformanceSubsystem.h"

const FString UPerformanceSubsystem::GetCPUName() const
{
    return FWindowsPlatformMisc::GetCPUBrand();
}

const FString UPerformanceSubsystem::GetCPUBrand() const
{
    return FWindowsPlatformMisc::GetCPUVendor();
}

const FString UPerformanceSubsystem::GetGPUName() const
{
    return FWindowsPlatformMisc::GetPrimaryGPUBrand();
}

const FString UPerformanceSubsystem::GetOSVersion() const
{
    return FWindowsPlatformMisc::GetOSVersion();
}

const FString UPerformanceSubsystem::GetUsedPhysicalMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float UsedPhysicalMemory = (float)MemoryStats.UsedPhysical / (1024 * 1024 * 1024);
    FString UsedPhysicalString = FString::Printf(TEXT("%.2f GB"), UsedPhysicalMemory);
    return UsedPhysicalString;
}

const FString UPerformanceSubsystem::GetAvailablePhysicalMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float AvailablePhysicalMemory = (float)MemoryStats.AvailablePhysical / (1024 * 1024 * 1024);
    FString UsedAvailablePhysicalMemory = FString::Printf(TEXT("%.2f GB"), AvailablePhysicalMemory);
    return UsedAvailablePhysicalMemory;
}

const FString UPerformanceSubsystem::GetAvailableVirtualMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float AvailableVirtualMemory = (float)MemoryStats.AvailableVirtual / (1024 * 1024 * 1024);
    FString UsedAvailableVirtualMemory = FString::Printf(TEXT("%.2f GB"), AvailableVirtualMemory);
    return UsedAvailableVirtualMemory;
}

const FString UPerformanceSubsystem::GetUsedVirtualMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float UsedVirtualMemory = (float)MemoryStats.UsedVirtual / (1024 * 1024 * 1024);
    FString UsedVirtualMemoryString = FString::Printf(TEXT("%.2f GB"), UsedVirtualMemory);
    return UsedVirtualMemoryString;
}

const FString UPerformanceSubsystem::GetPeakUsedVirtualMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float PeakUsedVirtualMemory = (float)MemoryStats.PeakUsedVirtual / (1024 * 1024 * 1024);
    FString PeakUsedVirtualMemoryString = FString::Printf(TEXT("%.2f GB"), PeakUsedVirtualMemory);
    return PeakUsedVirtualMemoryString;
}

const FString UPerformanceSubsystem::GetPeakUsedPhysicalMemory() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    float PeakUsedPhysicalMemory = (float)MemoryStats.PeakUsedPhysical / (1024 * 1024 * 1024);
    FString PeakUsedPhysicalMemoryString = FString::Printf(TEXT("%.2f GB"), PeakUsedPhysicalMemory);
    return PeakUsedPhysicalMemoryString;
}