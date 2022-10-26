/**
 *--------------------------------------------------------------------------------------
 * Ocean.h
 *--------------------------------------------------------------------------------------
 * Ocean is a framework that aims towards creating a platform that can
 *   used for creating performance-critical demos & presentations, while
 *   having a simple and descriptive API.
 * 
 * The GitHub project can be found at: 'https://github.com/avramtraian/Ocean'.
 */

/**
 * MIT License
 * 
 * Copyright(c) 2022 Avram Traian
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifndef OC_IMPLEMENTATION
	#define OC_IMPLEMENTATION          0
#endif // OC_IMPLEMENTATION

#ifdef _WIN32
	#define OC_PLATFORM_WINDOWS        1
	#ifdef _WIN64
		#define OC_PLATFORM_WINDOWS_64 1
	#else
		#define OC_PLATFORM_WINDOWS_32 1
	#endif // _WIN64
#endif // _WIN32

#ifndef OC_PLATFORM_WINDOWS
	#define OC_PLATFORM_WINDOWS        0
#endif // OC_PLATFORM_WINDOWS
#ifndef OC_PLATFORM_WINDOWS_32
	#define OC_PLATFORM_WINDOWS_32     0
#endif // OC_PLATFORM_WINDOWS_32
#ifndef OC_PLATFORM_WINDOWS_64
	#define OC_PLATFORM_WINDOWS_64     0
#endif // OC_PLATFORM_WINDOWS_64

#ifdef _MSC_BUILD
	#define OC_COMPILER_MSVC           1
#endif // _MSC_BUILD
#ifndef OC_COMPILER_MSVC
	#define OC_COMPILER_MSVC           0
#endif // OC_COMPILER_MSVC

#if OC_COMPILER_MSVC
	#define OC_INLINE       __forceinline
	#define OC_DEBUGBREAK() __debugbreak()

	#define OC_FUNCTION     __FUNCSIG__
#endif

#define OC_NODISCARD    [[nodiscard]]
#define OC_MAYBE_UNUSED [[maybe_unused]]

#define OC_FILE __FILE__
#define OC_LINE __LINE__

#define OC_CONFIGURATION_ALREADY_DEFINED 0
#ifdef OC_DEBUG
	#if OC_DEBUG
		#undef OC_CONFIGURATION_ALREADY_DEFINED
		#define OC_CONFIGURATION_ALREADY_DEFINED 1
	#endif // OC_DEBUG
#endif // OC_DEBUG

#ifdef OC_RELEASE
	#if OC_RELEASE
		#undef OC_CONFIGURATION_ALREADY_DEFINED
		#define OC_CONFIGURATION_ALREADY_DEFINED 1
	#endif // OC_RELEASE
#endif // OC_RELEASE

#if !OC_CONFIGURATION_ALREADY_DEFINED
	#ifdef _DEBUG
		#define OC_DEBUG   1
	#else
		#define OC_RELEASE 1
	#endif
#endif
#undef OC_CONFIGURATION_ALREADY_DEFINED

#ifndef OC_DEBUG
	#define OC_DEBUG   0
#endif // OC_DEBUG
#ifndef OC_RELEASE
	#define OC_RELEASE 0
#endif // OC_RELEASE

#if OC_PLATFORM_WINDOWS_32

/**
 * Windows, 32-bit, platform types.
 */
struct FWindows32PlatformTypes
{
	using uint8  = unsigned __int8;
	using uint16 = unsigned __int16;
	using uint32 = unsigned __int32;
	using uint64 = unsigned __int64;
	using int8   = signed __int8;
	using int16  = signed __int16;
	using int32  = signed __int32;
	using int64  = signed __int64;
	using SizeT  = unsigned __int32;
};
using FPlatformTypes = FWindows32PlatformTypes;

#elif OC_PLATFORM_WINDOWS_64

 /**
  * Windows, 64-bit, platform types.
  */
struct FWindows64PlatformTypes
{
	using uint8  = unsigned __int8;
	using uint16 = unsigned __int16;
	using uint32 = unsigned __int32;
	using uint64 = unsigned __int64;
	using int8   = signed __int8;
	using int16  = signed __int16;
	using int32  = signed __int32;
	using int64  = signed __int64;
	using SizeT  = unsigned __int64;
};
using FPlatformTypes = FWindows64PlatformTypes;

#else
	#error Unknown/Unsupported platform!
#endif

/** An 8-bit Integer. Unsigned. */
using uint8 = FPlatformTypes::uint8;

/** A 16-bit Integer. Unsigned. */
using uint16 = FPlatformTypes::uint16;

/** A 32-bit Integer. Unsigned. */
using uint32 = FPlatformTypes::uint32;

/** A 64-bit Integer. Unsigned. */
using uint64 = FPlatformTypes::uint64;

/** An 8-bit Integer. Signed. */
using int8 = FPlatformTypes::int8;

/** A 16-bit Integer. Signed. */
using int16 = FPlatformTypes::int16;

/** A 32-bit Integer. Signed. */
using int32 = FPlatformTypes::int32;

/** A 64-bit Integer. Signed. */
using int64 = FPlatformTypes::int64;

/** An Unsigned Integer. 32 or 64-bit, depending on the system's architecture. */
using SizeT = FPlatformTypes::SizeT;

#if OC_DEBUG
	#define OC_ENABLE_CHECKS   1
	#define OC_ENABLE_VERIFIES 1
#elif OC_RELEASE
	#define OC_ENABLE_CHECKS   0
	#define OC_ENABLE_VERIFIES 0
#endif

#ifndef OC_USE_CHECKS_IN_RELEASE
	#define OC_USE_CHECKS_IN_RELEASE   0
#endif // OC_USE_CHECKS_IN_RELEASE
#ifndef OC_USE_VERIFIES_IN_RELEASE
	#define OC_USE_VERIFIES_IN_RELEASE 0
#endif // OC_USE_VERIFIES_IN_RELEASE

#if OC_RELEASE
	#if OC_USE_CHECKS_IN_RELEASE
		#undef OC_ENABLE_CHECKS
		#define OC_ENABLE_CHECKS   1
	#endif // OC_USE_CHECKS_IN_RELEASE
	#if OC_USE_VERIFIES_IN_RELEASE
		#undef OC_ENABLE_VERIFIES
		#define OC_ENABLE_VERIFIES 1
	#endif // OC_USE_VERIFIES_IN_RELEASE
#endif // OC_RELEASE

namespace OC
{

void OnCheckFailed(const char* Expression, const char* File, const char* Function, uint32 Line, const char* Message);

} // namespace OC

#if OC_ENABLE_CHECKS

/**
 * Checks if the expression is evaluated to true. If it is, nothing else is performed.
 *   Otherwise, an assert will be issued and the debugger will break on the line where the
 *   macro is located. After that, the application will force-stop.
 * This macro is enabled only in Debug builds, or if 'OC_USE_CHECKS_IN_RELEASE' is set to 1.
 *   Otherwise, it is defined as nothing (excluded from build).
 * If you want to provide a custom message, use 'Checkf' instead.
 * 
 * @param EXPRESSION The expression to evaluate.
 */
#define Check(EXPRESSION)                                                         \
	if (!(EXPRESSION))                                                            \
	{                                                                             \
		::OC::OnCheckFailed(#EXPRESSION, OC_FILE, OC_FUNCTION, OC_LINE, nullptr); \
		OC_DEBUGBREAK();                                                          \
	}

/**
 * Checks if the expression is evaluated to true. If it is, nothing else is performed.
 *   Otherwise, an assert will be issued and the debugger will break on the line where the
 *   macro is located. After that, the application will force-stop.
 * This macro is enabled only in Debug builds, or if 'OC_USE_CHECKS_IN_RELEASE' is set to 1.
 *   Otherwise, it is defined as nothing (excluded from build).
 * An message must be provided. If you don't want to, use 'Check' instead.
 * 
 * @param EXPRESSION The expression to evaluate.
 * @param ... The message.
 */
#define Checkf(EXPRESSION, ...)                                                       \
	if (!(EXPRESSION))                                                                \
	{                                                                                 \
		char buffer[512] = {};                                                        \
		sprintf_s(buffer, __VA_ARGS__);                                               \
		::OC::OnCheckFailed(#EXPRESSION, OC_FILE, OC_FUNCTION, OC_LINE, buffer);      \
		OC_DEBUGBREAK();                                                              \
	}

/**
 * Runs a sequence of code inside an enclosed scope.
 * This macro is enabled only in Debug builds, or if 'OC_USE_CHECKS_IN_RELEASE' is set to 1.
 *   Otherwise, it is defined as nothing (excluded from build).
 * 
 * @param ... The sequence of code to run.
 */
#define CheckCode(...) \
	{                  \
		__VA_ARGS__    \
	}

#else

#define Check(EXPRESSION)       // Ignore from build.
#define Checkf(EXPRESSION, ...) // Ignore from build.
#define CheckCode(...)          // Ignore from build.

#endif // OC_ENABLE_CHECKS

#if OC_ENABLE_VERIFIES

/**
 * Same as 'Check', but when this macro is disabled, the expression is run without performing any checks.
 * @see 'Check'.
 */
#define Verify(EXPRESSION)                                                        \
	if (!(EXPRESSION))                                                            \
	{                                                                             \
		::OC::OnCheckFailed(#EXPRESSION, OC_FILE, OC_FUNCTION, OC_LINE, nullptr); \
		OC_DEBUGBREAK();                                                          \
	}

/**
 * Same as 'Check', but when this macro is disabled, the expression is run without performing any checks.
 * @see 'Check'.
 */
#define Verifyf(EXPRESSION, ...)                                                      \
	if (!(EXPRESSION))                                                                \
	{                                                                                 \
		char buffer[512] = {};                                                        \
		sprintf_s(buffer, __VA_ARGS__);                                               \
		::OC::OnCheckFailed(#EXPRESSION, OC_FILE, OC_FUNCTION, OC_LINE, buffer);      \
		OC_DEBUGBREAK();                                                              \
	}

#else

#define Verify(EXPRESSION) EXPRESSION       // Run the expression, without any checks.
#define Verifyf(EXPRESSION, ...) EXPRESSION	// Run the expression, without any checks.

#endif // OC_ENABLE_VERIFIES

namespace OC
{

class FPlatform
{
public:
	static bool Initialize();
	static void Shutdown();

public:
	static void* AllocMemory(SizeT BlockSize);
	static void FreeMemory(void* MemoryBlock);

public:
	enum class EConsoleColor
	{
		Black    = 0,  Blue        = 1,  Green       = 2,  Aqua       = 3,
		Red      = 4,  Purple      = 5,  Yellow      = 6,  White      = 7,
		Gray     = 8,  LightBlue   = 9,  LightGreen  = 10, LightAqua  = 11,
		LightRed = 12, LightPurple = 13, LightYellow = 14, LightWhite = 15
	};

public:
	static bool SetConsoleColor(EConsoleColor TextColor, EConsoleColor BackgroundColor);
	static bool WriteToConsole(const char* Message, SizeT MessageSize);
};

} // namespace OC

/**
 *--------------------------------------
 * Structure for all memory operations.
 *--------------------------------------
 */
class FMemory
{
public:
	/**
	 * 
	 */
	OC_NODISCARD static void* AllocateRaw(SizeT BlockSize);

	/**
	 * 
	 */
	OC_NODISCARD static void* Allocate(SizeT BlockSize);

	/**
	 * 
	 */
	OC_NODISCARD static void* AllocateTagged(SizeT BlockSize);

	/**
	 * 
	 */
	static void FreeRaw(void* MemoryBlock);

	/**
	 * 
	 */
	static void Free(void* MemoryBlock);

public:
	/**
	 * Copies a number of bytes from the location pointed by the Source pointer to the location
	 *   pointed by the Destination pointer.
	 * The underlying types of pointers are irrelevant. The result is simply a binary copy of the data.
	 * To avoid overflows, both Destination and Source should point to memory blocks of at least Size
	 *   bytes. Also, the memory blocks should not overlap.
	 * 
	 * @param Destination Pointer to the block of memory where the bytes will be copied to.
	 * @param Source Pointer to the block of memory where the bytes will be copied from.
	 * @param Size The length (in bytes) of the copy.
	 */
	static void Copy(void* Destination, const void* Source, SizeT Size);

	/**
	 * Sets a number of bytes from the location pointed by the Destination pointer to the specified value.
	 * 
	 * @param Destination Pointer to the block of memory where the bytes will be set. To avoid overflow,
	 *   the memory block should be at least Size bytes long.
	 * @param Value The value to set each byte to.
	 * @param Size The number of bytes to set.
	 */
	static void Set(void* Destination, uint8 Value, SizeT Size);

	/**
	 * Sets a block of memory to 0.
	 * 
	 * @param Destination Pointer to the block of memory to set to 0. To avoid overflow, the memory block
	 *   should be at least Size bytes long.
	 * @param Size The number of bytes to set to 0.
	 */
	static void Zero(void* Destination, SizeT Size);

public:
	static bool Initialize();
	static void Shutdown();
};

namespace OC
{

class FLogger
{
public:
	enum class ELogType
	{
		Debug, Trace, Info, Warn, Error, Fatal
	};

public:
	static void Write(const char* Tag, ELogType LogType, const char* Message, ...);
};

} // namespace OC

#if OC_DEBUG
	#define OC_ENABLE_DEBUG_LOGS 1
	#define OC_ENABLE_TRACE_LOGS 1
	#define OC_ENABLE_INFO_LOGS  1
	#define OC_ENABLE_WARN_LOGS  1
	#define OC_ENABLE_ERROR_LOGS 1
	#define OC_ENABLE_FATAL_LOGS 1
#elif OC_RELEASE
	#define OC_ENABLE_DEBUG_LOGS 0
	#define OC_ENABLE_TRACE_LOGS 0
	#define OC_ENABLE_INFO_LOGS  0
	#define OC_ENABLE_WARN_LOGS  0
	#define OC_ENABLE_ERROR_LOGS 1
	#define OC_ENABLE_FATAL_LOGS 1
#endif

#ifndef OC_USE_DEBUG_LOGS_IN_RELEASE
	#define OC_USE_DEBUG_LOGS_IN_RELEASE 0
#endif // OC_USE_DEBUG_LOGS_IN_RELEASE
#ifndef OC_USE_TRACE_LOGS_IN_RELEASE
	#define OC_USE_TRACE_LOGS_IN_RELEASE 0
#endif // OC_USE_TRACE_LOGS_IN_RELEASE
#ifndef OC_USE_INFO_LOGS_IN_RELEASE
	#define OC_USE_INFO_LOGS_IN_RELEASE  0
#endif // OC_USE_INFO_LOGS_IN_RELEASE
#ifndef OC_USE_WARN_LOGS_IN_RELEASE
	#define OC_USE_WARN_LOGS_IN_RELEASE  0
#endif // OC_USE_WARN_LOGS_IN_RELEASE

#if OC_RELEASE
	#if OC_USE_DEBUG_LOGS_IN_RELEASE
		#undef OC_ENABLE_DEBUG_LOGS
		#define OC_ENABLE_DEBUG_LOGS 1
	#endif // OC_USE_DEBUG_LOGS_IN_RELEASE
	#if OC_USE_TRACE_LOGS_IN_RELEASE
		#undef OC_ENABLE_TRACE_LOGS
		#define OC_ENABLE_TRACE_LOGS 1
	#endif // OC_USE_DEBUG_LOGS_IN_RELEASE
	#if OC_USE_INFO_LOGS_IN_RELEASE
		#undef OC_ENABLE_INFO_LOGS
		#define OC_ENABLE_INFO_LOGS  1
	#endif // OC_USE_DEBUG_LOGS_IN_RELEASE
	#if OC_USE_WARN_LOGS_IN_RELEASE
		#undef OC_ENABLE_WARN_LOGS
		#define OC_ENABLE_WARN_LOGS  1
	#endif // OC_USE_DEBUG_LOGS_IN_RELEASE
#endif // OC_RELEASE

#if OC_ENABLE_DEBUG_LOGS
#define OC_CORE_DEBUG(...)     ::OC::FLogger::Write("CORE", ::OC::FLogger::ELogType::Debug, __VA_ARGS__)
#define OC_TAG_DEBUG(TAG, ...) ::OC::FLogger::Write(TAG,    ::OC::FLogger::ELogType::Debug, __VA_ARGS__)
#define OC_GAME_DEBUG(...)     ::OC::FLogger::Write("GAME", ::OC::FLogger::ELogType::Debug, __VA_ARGS__)
#else
#define OC_CORE_DEBUG(...)     // Ignore from build.
#define OC_TAG_DEBUG(TAG, ...) // Ignore from build.
#define OC_GAME_DEBUG(...)     // Ignore from build.
#endif // OC_ENABLE_DEBUG_LOGS

#if OC_ENABLE_TRACE_LOGS
#define OC_CORE_TRACE(...)     ::OC::FLogger::Write("CORE", ::OC::FLogger::ELogType::Trace, __VA_ARGS__)
#define OC_TAG_TRACE(TAG, ...) ::OC::FLogger::Write(TAG,    ::OC::FLogger::ELogType::Trace, __VA_ARGS__)
#define OC_GAME_TRACE(...)     ::OC::FLogger::Write("GAME", ::OC::FLogger::ELogType::Trace, __VA_ARGS__)
#else
#define OC_CORE_TRACE(...)     // Ignore from build.
#define OC_TAG_TRACE(TAG, ...) // Ignore from build.
#define OC_GAME_TRACE(...)     // Ignore from build.
#endif // OC_ENABLE_TRACE_LOGS

#if OC_ENABLE_INFO_LOGS
#define OC_CORE_INFO(...)     ::OC::FLogger::Write("CORE", ::OC::FLogger::ELogType::Info, __VA_ARGS__)
#define OC_TAG_INFO(TAG, ...) ::OC::FLogger::Write(TAG,    ::OC::FLogger::ELogType::Info, __VA_ARGS__)
#define OC_GAME_INFO(...)     ::OC::FLogger::Write("GAME", ::OC::FLogger::ELogType::Info, __VA_ARGS__)
#else
#define OC_CORE_INFO(...)     // Ignore from build.
#define OC_TAG_INFO(TAG, ...) // Ignore from build.
#define OC_GAME_INFO(...)     // Ignore from build.
#endif // OC_ENABLE_INFO_LOGS

#if OC_ENABLE_WARN_LOGS
#define OC_CORE_WARN(...)     ::OC::FLogger::Write("CORE", ::OC::FLogger::ELogType::Warn, __VA_ARGS__)
#define OC_TAG_WARN(TAG, ...) ::OC::FLogger::Write(TAG,    ::OC::FLogger::ELogType::Warn, __VA_ARGS__)
#define OC_GAME_WARN(...)     ::OC::FLogger::Write("GAME", ::OC::FLogger::ELogType::Warn, __VA_ARGS__)
#else
#define OC_CORE_WARN(...)     // Ignore from build.
#define OC_TAG_WARN(TAG, ...) // Ignore from build.
#define OC_GAME_WARN(...)     // Ignore from build.
#endif // OC_ENABLE_WARN_LOGS

#if OC_ENABLE_ERROR_LOGS
#define OC_CORE_ERROR(...)     ::OC::FLogger::Write("CORE", ::OC::FLogger::ELogType::Error, __VA_ARGS__)
#define OC_TAG_ERROR(TAG, ...) ::OC::FLogger::Write(TAG,    ::OC::FLogger::ELogType::Error, __VA_ARGS__)
#define OC_GAME_ERROR(...)     ::OC::FLogger::Write("GAME", ::OC::FLogger::ELogType::Error, __VA_ARGS__)
#else
#define OC_CORE_ERROR(...)     // Ignore from build.
#define OC_TAG_ERROR(TAG, ...) // Ignore from build.
#define OC_GAME_ERROR(...)     // Ignore from build.
#endif // OC_ENABLE_ERROR_LOGS

#if OC_ENABLE_FATAL_LOGS
#define OC_CORE_FATAL(...)     ::OC::FLogger::Write("CORE", ::OC::FLogger::ELogType::Fatal, __VA_ARGS__)
#define OC_TAG_FATAL(TAG, ...) ::OC::FLogger::Write(TAG,    ::OC::FLogger::ELogType::Fatal, __VA_ARGS__)
#define OC_GAME_FATAL(...)     ::OC::FLogger::Write("GAME", ::OC::FLogger::ELogType::Fatal, __VA_ARGS__)
#else
#define OC_CORE_FATAL(...)     // Ignore from build.
#define OC_TAG_FATAL(TAG, ...) // Ignore from build.
#define OC_GAME_FATAL(...)     // Ignore from build.
#endif // OC_ENABLE_FATAL_LOGS

#pragma region Math Utilities

/**
 * The most common math constants.
 */
#define PI                        (3.1415926535897932F)
#define KINDA_SMALL_NUMBER        (1e-4F)
#define SMALL_NUMBER              (1e-8F)
#define BIG_NUMBER                (3.4e38F)
#define EULUERS_NUMBER            (2.7182818284590452F)
#define GOLDEN_RATIO              (1.6180339887498948F)

#define DOUBLE_PI                 (3.1415926535897932384626433832795028)
#define DOUBLE_KINDA_SMALL_NUMBER (1e-4)
#define DOUBLE_SMALL_NUMBER       (1e-8)
#define DOUBLE_BIG_NUMBER         (3.4e+38)
#define DOUBLE_EULUERS_NUMBER     (2.7182818284590452353602874713526624)
#define DOUBLE_GOLDEN_RATIO       (1.6180339887498948482045868343656381)

/**
* More PI-related constants.
*/
#define INV_PI                    (0.31830988618F)
#define TWO_PI                    (6.28318530717F)
#define HALF_PI                   (1.57079632679F)
#define PI_SQUARED                (9.86960440108F)

#define DOUBLE_INV_PI             (0.31830988618379067154)
#define DOUBLE_TWO_PI             (6.28318530717958647692)
#define DOUBLE_HALF_PI            (1.57079632679489661923)
#define DOUBLE_PI_SQUARED         (9.86960440108935861883)

/**
* Common square roots.
*/
#define SQRT_2                    (1.41421356237F)
#define SQRT_3                    (1.73205080756F)
#define INV_SQRT_2                (0.70710678118F)
#define INV_SQRT_3                (0.57735026918F)
#define HALF_SQRT_2               (0.70710678118F)
#define HALF_SQRT_3               (0.86602540378F)

#define DOUBLE_SQRT_2             (1.4142135623730950488016887242097)
#define DOUBLE_SQRT_3             (1.7320508075688772935274463415059)
#define DOUBLE_INV_SQRT_2         (0.7071067811865475244008443621048)
#define DOUBLE_INV_SQRT_3         (0.5773502691896257645091487805019)
#define DOUBLE_HALF_SQRT_2        (0.7071067811865475244008443621048)
#define DOUBLE_HALF_SQRT_3        (0.8660254037844386467637231707529)

/**
* Structure for all math utility and helper functions.
*/
struct FMath
{
public:
	/**
	 * Gets the absolute value of a number.
	 * 
	 * @param The number to get its absolute value.
	 * 
	 * @return The absolute value.
	 */
	template<typename T>
	static OC_INLINE T Abs(T X)
	{
		return X < T(0) ? -X : X;
	}

	/**
	 * Gets the minimum between two numbers.
	 * 
	 * @param A The first number.
	 * @param B The second number.
	 * 
	 * @return The minimum.
	 */
	template<typename T>
	static OC_INLINE T Min(T A, T B)
	{
		return A < B ? A : B;
	}

	/**
	 * Gets the number with the smallest absolute value.
	 * It can be interpreted as getting the number which is the closest
	 *   to the number's axis origin.
	 * 
	 * @param A The first number.
	 * @param B The second number.
	 * 
	 * @return The number with the smallest absolute value.
	 */
	template<typename T>
	static OC_INLINE T MinAbs(T A, T B)
	{
		return FMath::Min(FMath::Abs(A), FMath::MaxAbs(B));
	}

	/**
	 * Gets the maximum between two numbers.
	 * 
	 * @param A The first number.
	 * @param B The second number.
	 * 
	 * @return The maximum.
	 */
	template<typename T>
	static OC_INLINE T Max(T A, T B)
	{
		return A > B ? A : B;
	}

	/**
	 * Gets the number with the biggest absolute value.
	 * It can be interpreted as getting the number which is the furthest
	 *   to the number's axis origin.
	 * 
	 * @param A The first number.
	 * @param B The second number.
	 *
	 * @return The number with the biggest absolute value.
	 */
	template<typename T>
	static OC_INLINE T MaxAbs(T A, T B)
	{
		return FMath::Max(FMath::Abs(A), FMath::MaxAbs(B));
	}

	/**
	 * Clamps a value between two numbers; [MinRange, MaxRange].
	 * 
	 * @param X The number to clamp.
	 * @param MinRange The lower part of the range.
	 * @param MaxRange The upper part of the range.
	 * 
	 * @return The clamped value.
	 */
	template<typename T>
	static OC_INLINE T Clamp(T X, T MinRange, T MaxRange)
	{
		return FMath::Min(MaxRange, FMath::Max(MinRange, X));
	}

public:
	/**
	 * Calculates a number's square root.
	 * 
	 * @param X The number to calculate its square root.
	 * 
	 * @return The square root.
	 */
	static float Sqrt(float X);

	/**
	 * @see 'FMath::Sqrt(float)'.
	 */
	static double Sqrt(double X);
};

// Math Utilities
#pragma endregion

namespace OC
{

/** Forward declaration. */
template<typename T>
struct TVector2;

/** Forward declaration. */
template<typename T>
struct TVector3;

/** Forward declaration. */
template<typename T>
struct TVector4;

} // namespace OC

#pragma region TVector2 Declaration

namespace OC
{

/**
 *-------------------------------------------------------------
 * A 2-component (X, Y) vector, with floating point precision.
 *-------------------------------------------------------------
 */
template<typename T>
struct TVector2
{
public:
	/** The vector's X component. */
	T X;

	/** The vector's X component. */
	T Y;

public:
	/** @return A vector with the components (0, 0). */
	static constexpr TVector2<T> Zero() { return TVector2<T>(T(0.0), T(0.0)); }

	/** @return A vector with the components (1, 1). */
	static constexpr TVector2<T> One() { return TVector2<T>(T(1.0), T(1.0)); }

	/** @return A vector with the components (1, 0). */
	static constexpr TVector2<T> UnitX() { return TVector2<T>(T(1.0), T(0.0)); }

	/** @return A vector with the components (0, 1). */
	static constexpr TVector2<T> UnitY() { return TVector2<T>(T(0.0), T(1.0)); }

public:
	/**
	 * Default constructor.
	 * Initializes all components with 0.
	 */
	OC_INLINE TVector2();

	/**
	 * Copy constructor.
	 * Initializes the components with the other's.
	 *
	 * @param Other The vector to copy.
	 */
	OC_INLINE TVector2(const TVector2<T>& Other);

	/**
	 * Constructor using an initializing value for each component.
	 *
	 * @param InX Initializing value for the X component.
	 * @param InY Initializing value for the Y component.
	 */
	OC_INLINE TVector2(T InX, T InY);

	/**
	 * Constructor using a single value to initialize all components.
	 *
	 * @param Scalar Initializing value for the components.
	 */
	OC_INLINE explicit TVector2(T Scalar);

	/**
	 * Constructor using a 3-component vector, using just the X and Y
	 *   components. The Z-component is ignored/unused.
	 *
	 * @param Vector3 The 3-component vector.
	 */
	OC_INLINE explicit TVector2(const TVector3<T>& Vector3);

	/**
	 * Constructor using a 4-component vector, using just the X and Y
	 *   components. The Z and W components are ignored/unused.
	 *
	 * @param Vector4 The 4-component vector.
	 */
	OC_INLINE explicit TVector2(const TVector4<T>& Vector4);

	/**
	 * Copy/Assign operator.
	 *
	 * @param Other The vector to copy.
	 *
	 * @return Reference to this, after the copy.
	 */
	OC_INLINE TVector2<T>& operator=(const TVector2<T>& Other);

public:
	/**
	 * Addition operator. Adds two vectors.
	 *
	 * @param Other The vector to add.
	 *
	 * @return The result of the addition.
	 */
	OC_INLINE TVector2<T> operator+(const TVector2<T>& Other) const;

	/**
	 * Addition operator. Adds a vector to this.
	 *
	 * @param Other The vector to add.
	 *
	 * @return A reference to this, after the addition.
	 */
	OC_INLINE TVector2<T>& operator+=(const TVector2<T>& Other);

	/**
	 * Subtraction operator. Subtracts two vectors.
	 *
	 * @param Other The vector to subtract.
	 *
	 * @return The result of the subtraction.
	 */
	OC_INLINE TVector2<T> operator-(const TVector2<T>& Other) const;

	/**
	 * Subtraction operator. Subtracts a vector from this.
	 *
	 * @param Other The vector to subtract.
	 *
	 * @return A reference to this, after the subtraction.
	 */
	OC_INLINE TVector2<T>& operator-=(const TVector2<T>& Other);

	/**
	 * Multiplication operator. Multiplies a scalar to this.
	 *
	 * @param Scalar The scalar to multiply.
	 *
	 * @return The result of the multiplication.
	 */
	OC_INLINE TVector2<T> operator*(T Scalar) const;

	/**
	 * Multiplication operator. Multiplies this with a scalar.
	 *
	 * @param Scalar The scalar to multiply.
	 *
	 * @return A reference to this, after the multiplication.
	 */
	OC_INLINE TVector2<T>& operator*=(T Scalar);

public:
	/** @return The square of the vector's length. */
	OC_INLINE T LengthSquared() const;

	/** @return The vector's length. */
	OC_INLINE T Length() const;

	/**
	 * Calculates the dot product between two vectors.
	 *
	 * @param A The first vector.
	 * @param B The second vector.
	 *
	 * @return The dot product.
	 */
	OC_INLINE static T DotProduct(const TVector2<T>& A, TVector2<T>& B);

	/**
	 * Calculates the dot product between this and another vector.
	 *
	 * @param Other The vector to calculate the dot product with.
	 *
	 * @return The dot product.
	 */
	OC_INLINE T Dot(const TVector2<T>& Other) const;

	/**
	 * Calculates the dot product between this and another vector.
	 *
	 * @param Other The vector to calculate the dot product with.
	 *
	 * @return The dot product.
	 */
	OC_INLINE T operator|(const TVector2<T>& Other) const;

	/**
	 * Checks if a vector is normalized (has length 1).
	 *
	 * @param Tolerance The maximum allowed difference between the squared length
	 *   of the vector and 1, in order to consider the vector as normalized. Default
	 *   value of 'KINDA_SMALL_NUMBER'.
	 *
	 * @return True if the vector is normalized; False otherwise.
	 */
	OC_INLINE bool IsNormalized(T Tolerance = KINDA_SMALL_NUMBER) const;

	/**
	 * Calculates the normal for this vector.
	 * The normal is a vector with the same direction as this, but unit length (1).
	 *
	 * @return The vector's normal.
	 */
	OC_INLINE TVector2<T> GetNormal() const;

	/**
	 * Calculates the normal for this vector, only if the vector is not already normalized.
	 *
	 * @param Tolerance The maximum allowed difference between the squared length
	 *   of the vector and 1, in order to consider the vector as already normalized. Default
	 *   value of 'KINDA_SMALL_NUMBER'.
	 *
	 * @return The vector's normal.
	 */
	OC_INLINE TVector2<T> GetNormalIf(T Tolerance = KINDA_SMALL_NUMBER) const;

	/**
	 * Calculates the normal for this vector, only if it is possible.
	 *
	 * @param Threshold The maximum value for the vector's squared length for which the vector's
	 *   normal is undefined (as the length is close to 0).
	 * @param ResultIfError The vector that will be returned if the normal is undefined.
	 *   NOTE: ResultIfError should already be a normalized vector.
	 *
	 * @return The vector's normal, if possible; Otherwise, 'ResultIfError'.
	 */
	OC_INLINE TVector2<T> GetSafeNormal(T Threshold, const TVector2<T>& ResultIfError) const;

	/**
	 * Calculates the normal for this vector, only if it is possible and the vector is not already
	 *   normalized.
	 *
	 * @param Threshold The maximum value for the vector's squared length for which the vector's
	 *   normal is undefined (as the length is close to 0).
	 * @param ResultIfError The vector that will be returned if the normal is undefined.
	 *   NOTE: ResultIfError should already be a normalized vector.
	 * @param Tolerance The maximum allowed difference between the squared length
	 *   of the vector and 1, in order to consider the vector as already normalized. Default
	 *   value of 'KINDA_SMALL_NUMBER'.
	 *
	 * @return The vector's normal, if possible; Otherwise, 'ResultIfError'.
	 */
	OC_INLINE TVector2<T> GetSafeNormalIf(T Threshold, const TVector2<T>& ResultIfError, T Tolerance) const;
};

}

/**
*------------------------------------------------------------
* A 2-component vector with single-floating point precision.
*------------------------------------------------------------
* @see 'TVector2<T>'.
*/
using FVector2 = OC::TVector2<float>;

// TVector2 Declaration
#pragma endregion

#pragma region TVector3 Declaration

namespace OC
{

/**
 *----------------------------------------------------------------
 * A 3-component (X, Y, Z) vector, with floating point precision.
 *----------------------------------------------------------------
 */
template<typename T>
struct TVector3
{
public:
	/** The vector's X component. */
	T X;

	/** The vector's X component. */
	T Y;

	/** The vector's X component. */
	T Z;

public:
	/** @return A vector with the components (0, 0, 0). */
	static constexpr TVector3<T> Zero() { return TVector3<T>(T(0.0), T(0.0), T(0.0)); }

	/** @return A vector with the components (1, 1, 1). */
	static constexpr TVector3<T> One() { return TVector3<T>(T(1.0), T(1.0), T(1.0)); }

	/** @return A vector with the components (1, 0, 0). */
	static constexpr TVector3<T> Forward() { return TVector3<T>(T(1.0), T(0.0), T(0.0)); }

	/** @return A vector with the components (-1, 0, 0). */
	static constexpr TVector3<T> Backward() { return TVector3<T>(T(-1.0), T(0.0), T(0.0)); }

	/** @return A vector with the components (0, 1, 0). */
	static constexpr TVector3<T> Right() { return TVector3<T>(T(0.0), T(1.0), T(0.0)); }

	/** @return A vector with the components (0, -1, 0). */
	static constexpr TVector3<T> Left() { return TVector3<T>(T(0.0), T(-1.0), T(0.0)); }

	/** @return A vector with the components (0, 0, 1). */
	static constexpr TVector3<T> Up() { return TVector3<T>(T(0.0), T(0.0), T(1.0)); }

	/** @return A vector with the components (0, 0, -1). */
	static constexpr TVector3<T> Down() { return TVector3<T>(T(0.0), T(0.0), T(-1.0)); }

	/** @return A vector with the components (1, 0, 0). */
	static constexpr TVector3<T> UnitX() { return TVector3<T>(T(1.0), T(0.0), T(0.0)); }

	/** @return A vector with the components (0, 1, 0). */
	static constexpr TVector3<T> UnitY() { return TVector3<T>(T(1.0), T(1.0), T(0.0)); }

	/** @return A vector with the components (0, 0, 1). */
	static constexpr TVector3<T> UnitZ() { return TVector3<T>(T(1.0), T(0.0), T(1.0)); }

public:
	/**
	 * Default constructor.
	 * Initializes all components with 0.
	 */
	OC_INLINE TVector3();

	/**
	 * Copy constructor.
	 * Initializes the components with the other's.
	 * 
	 * @param Other The vector to copy.
	 */
	OC_INLINE TVector3(const TVector3& Other);

	/**
	 * Constructor using an initializing value for each component.
	 * 
	 * @param InX Initializing value for the X component.
	 * @param InY Initializing value for the Y component.
	 * @param InZ Initializing value for the Z component.
	 */
	OC_INLINE TVector3(T InX, T InY, T InZ);

	/**
	 * Constructor using a single value to initialize all components.
	 * 
	 * @param Scalar Initializing value for the components.
	 */
	OC_INLINE explicit TVector3(T Scalar);

	/**
	 * Constructor using a 2-component vector (for X and Y). The Z-component
	 *   is initialized with 0.
	 * 
	 * @param Vector2 The 2-component vector.
	 */
	OC_INLINE explicit TVector3(const TVector2<T>& Vector2);

	/**
	 * Constructor using a 2-component vector (for X and Y) and a separate
	 *   value for the Z-component.
	 *
	 * @param Vector2 The 2-component vector.
	 * @param InZ The Z-component initializing value.
	 */
	OC_INLINE TVector3(const TVector2<T>& Vector2, T InZ);

	/**
	 * Constructor using a 4-component vector, using just the X, Y and Z
	 *   components. The W-component is ignored/unused.
	 *
	 * @param Vector4 The 4-component vector.
	 */
	OC_INLINE explicit TVector3(const TVector4<T>& Vector4);

	/**
	 * Copy/Assign operator.
	 * 
	 * @param Other The vector to copy.
	 * 
	 * @return Reference to this, after the copy.
	 */
	OC_INLINE TVector3<T>& operator=(const TVector3<T>& Other);

public:
	/**
	 * Addition operator. Adds two vectors.
	 * 
	 * @param Other The vector to add.
	 * 
	 * @return The result of the addition.
	 */
	OC_INLINE TVector3<T> operator+(const TVector3<T>& Other) const;

	/**
	 * Addition operator. Adds a vector to this.
	 *
	 * @param Other The vector to add.
	 *
	 * @return A reference to this, after the addition.
	 */
	OC_INLINE TVector3<T>& operator+=(const TVector3<T>& Other);

	/**
	 * Subtraction operator. Subtracts two vectors.
	 *
	 * @param Other The vector to subtract.
	 *
	 * @return The result of the subtraction.
	 */
	OC_INLINE TVector3<T> operator-(const TVector3<T>& Other) const;

	/**
	 * Subtraction operator. Subtracts a vector from this.
	 *
	 * @param Other The vector to subtract.
	 *
	 * @return A reference to this, after the subtraction.
	 */
	OC_INLINE TVector3<T>& operator-=(const TVector3<T>& Other);

	/**
	 * Multiplication operator. Multiplies a scalar to this.
	 *
	 * @param Scalar The scalar to multiply.
	 *
	 * @return The result of the multiplication.
	 */
	OC_INLINE TVector3<T> operator*(T Scalar) const;

	/**
	 * Multiplication operator. Multiplies this with a scalar.
	 *
	 * @param Scalar The scalar to multiply.
	 * 
	 * @return A reference to this, after the multiplication.
	 */
	OC_INLINE TVector3<T>& operator*=(T Scalar);

public:
	/** @return The square of the vector's length. */
	OC_INLINE T LengthSquared() const;

	/** @return The vector's length. */
	OC_INLINE T Length() const;

	/**
	 * Calculates the dot product between two vectors.
	 * 
	 * @param A The first vector.
	 * @param B The second vector.
	 * 
	 * @return The dot product.
	 */
	OC_INLINE static T DotProduct(const TVector3<T>& A, const TVector3<T>& B);

	/**
	 * Calculates the dot product between this and another vector.
	 *
	 * @param Other The vector to calculate the dot product with.
	 *
	 * @return The dot product.
	 */
	OC_INLINE T Dot(const TVector3<T>& Other) const;

	/**
	 * Calculates the dot product between this and another vector.
	 * 
	 * @param Other The vector to calculate the dot product with.
	 * 
	 * @return The dot product.
	 */
	OC_INLINE T operator|(const TVector3<T>& Other) const;

	/**
	 * Calculates the cross product between two vectors.
	 *
	 * @param A The first vector.
	 * @param B The second vector.
	 *
	 * @return The cross product.
	 */
	OC_INLINE static TVector3<T> CrossProduct(const TVector3<T>& A, const TVector3<T>& B);

	/**
	 * Calculates the cross product between this and another vector.
	 *
	 * @param Other The vector to calculate the cross product with.
	 *
	 * @return The cross product.
	 */
	OC_INLINE TVector3<T> Cross(const TVector3<T>& Other) const;

	/**
	 * Calculates the cross product between this and another vector.
	 *
	 * @param Other The vector to calculate the cross product with.
	 *
	 * @return The cross product.
	 */
	OC_INLINE TVector3<T> operator^(const TVector3<T>& Other) const;

	/**
	* Assigns to this the cross product between this and another vector.
	* 
	* @param Other The vector to calculate the cross product with.
	* 
	* @return Reference to this, after the assignment.
	*/
	OC_INLINE TVector3<T>& operator^=(const TVector3<T>& Other);

	/**
	 * Checks if a vector is normalized (has length 1).
	 * 
	 * @param Tolerance The maximum allowed difference between the squared length
	 *   of the vector and 1, in order to consider the vector as normalized. Default
	 *   value of 'KINDA_SMALL_NUMBER'.
	 * 
	 * @return True if the vector is normalized; False otherwise.
	 */
	OC_INLINE bool IsNormalized(T Tolerance = KINDA_SMALL_NUMBER) const;

	/**
	 * Calculates the normal for this vector.
	 * The normal is a vector with the same direction as this, but unit length (1).
	 * 
	 * @return The vector's normal.
	 */
	OC_INLINE TVector3<T> GetNormal() const;

	/**
	 * Calculates the normal for this vector, only if the vector is not already normalized.
	 * 
	 * @param Tolerance The maximum allowed difference between the squared length
	 *   of the vector and 1, in order to consider the vector as already normalized. Default
	 *   value of 'KINDA_SMALL_NUMBER'.
	 * 
	 * @return The vector's normal.
	 */
	OC_INLINE TVector3<T> GetNormalIf(T Tolerance = KINDA_SMALL_NUMBER) const;

	/**
	 * Calculates the normal for this vector, only if it is possible.
	 * 
	 * @param Threshold The maximum value for the vector's squared length for which the vector's
	 *   normal is undefined (as the length is close to 0).
	 * @param ResultIfError The vector that will be returned if the normal is undefined.
	 *   NOTE: ResultIfError should already be a normalized vector.
	 * 
	 * @return The vector's normal, if possible; Otherwise, 'ResultIfError'.
	 */
	OC_INLINE TVector3<T> GetSafeNormal(T Threshold, const TVector3<T>& ResultIfError) const;

	/**
	 * Calculates the normal for this vector, only if it is possible and the vector is not already
	 *   normalized.
	 *
	 * @param Threshold The maximum value for the vector's squared length for which the vector's
	 *   normal is undefined (as the length is close to 0).
	 * @param ResultIfError The vector that will be returned if the normal is undefined.
	 *   NOTE: ResultIfError should already be a normalized vector.
	 * @param Tolerance The maximum allowed difference between the squared length
	 *   of the vector and 1, in order to consider the vector as already normalized. Default
	 *   value of 'KINDA_SMALL_NUMBER'.
	 *
	 * @return The vector's normal, if possible; Otherwise, 'ResultIfError'.
	 */
	OC_INLINE TVector3<T> GetSafeNormalIf(T Threshold, const TVector3<T>& ResultIfError, T Tolerance) const;
};

} // namespace OC

/**
*------------------------------------------------------------
* A 3-component vector with single-floating point precision.
*------------------------------------------------------------
* @see 'TVector3<T>'.
*/
using FVector = OC::TVector3<float>;

// TVector3 Declaration
#pragma endregion

#pragma region TVector4 Declaration

namespace OC
{

/**
 *-------------------------------------------------------------------
 * A 4-component (X, Y, Z, W) vector, with floating point precision.
 *-------------------------------------------------------------------
 */
template<typename T>
struct TVector4
{
public:
	/** The vector's X component. */
	T X;

	/** The vector's X component. */
	T Y;

	/** The vector's Z component. */
	T Z;

	/** The vector's W component. */
	T W;

public:
	/** @return A vector with the components (0, 0, 0, 0). */
	static constexpr TVector4<T> Zero() { return TVector4<T>(T(0.0), T(0.0), T(0.0), T(0.0)); }

	/** @return A vector with the components (1, 1, 1, 1). */
	static constexpr TVector4<T> One() { return TVector4<T>(T(1.0), T(1.0), T(1.0), T(1.0)); }

	/** @return A vector with the components (1, 0, 0, 0). */
	static constexpr TVector4<T> UnitX() { return TVector4<T>(T(1.0), T(0.0), T(0.0), T(0.0)); }

	/** @return A vector with the components (0, 1, 0, 0). */
	static constexpr TVector4<T> UnitY() { return TVector4<T>(T(0.0), T(1.0), T(0.0), T(0.0)); }

	/** @return A vector with the components (0, 0, 1, 0). */
	static constexpr TVector4<T> UnitZ() { return TVector4<T>(T(0.0), T(0.0), T(1.0), T(0.0)); }

	/** @return A vector with the components (0, 0, 0, 1). */
	static constexpr TVector4<T> UnitW() { return TVector4<T>(T(0.0), T(0.0), T(0.0), T(1.0)); }

public:
	/**
	 * Default constructor.
	 * Initializes all components with 0.
	 */
	OC_INLINE TVector4();

	/**
	 * Copy constructor.
	 * Initializes the components with the other's.
	 *
	 * @param Other The vector to copy.
	 */
	OC_INLINE TVector4(const TVector4<T>& Other);

	/**
	 * Constructor using an initializing value for each component.
	 *
	 * @param InX Initializing value for the X component.
	 * @param InY Initializing value for the Y component.
	 * @param InZ Initializing value for the Z component.
	 * @param InW Initializing value for the W component.
	 */
	OC_INLINE TVector4(T InX, T InY, T InZ, T InW);

	/**
	 * Constructor using a single value to initialize all components.
	 *
	 * @param Scalar Initializing value for the components.
	 */
	OC_INLINE explicit TVector4(T Scalar);

	/**
	 * Constructor using a 2-component vector (for X and Y). The Z and W components
	 *   are initialized with 0.
	 *
	 * @param Vector2 The 2-component vector.
	 */
	OC_INLINE explicit TVector4(const TVector2<T>& Vector2);

	/**
	 * Constructor using a 2-component vector (for X and Y) and separate
	 *   values for the Z and W components.
	 *
	 * @param Vector2 The 2-component vector.
	 * @param InZ The Z-component initializing value.
	 * @param InW The W-component initializing value.
	 */
	OC_INLINE TVector4(const TVector2<T>& Vector2, T InZ, T InW);

	/**
	 * Constructor using a 3-component vector (for X, Y and Z). The W-component
	 *   is initialized with 0.
	 *
	 * @param Vector3 The 3-component vector.
	 */
	OC_INLINE explicit TVector4(const TVector3<T>& Vector3);

	/**
	 * Constructor using a 3-component vector (for X, Y and Z) and a separate
	 *   value for the W-component.
	 *
	 * @param Vector3 The 3-component vector.
	 * @param InW The W-component initializing value.
	 */
	OC_INLINE TVector4(const TVector3<T>& Vector3, T InW);

	/**
	 * Copy/Assign operator.
	 *
	 * @param Other The vector to copy.
	 *
	 * @return Reference to this, after the copy.
	 */
	OC_INLINE TVector4<T>& operator=(const TVector4<T>& Other);

public:
	/**
	 * Addition operator. Adds two vectors.
	 *
	 * @param Other The vector to add.
	 *
	 * @return The result of the addition.
	 */
	OC_INLINE TVector4<T> operator+(const TVector4<T>& Other) const;

	/**
	 * Addition operator. Adds a vector to this.
	 *
	 * @param Other The vector to add.
	 *
	 * @return A reference to this, after the addition.
	 */
	OC_INLINE TVector4<T>& operator+=(const TVector4<T>& Other);

	/**
	 * Subtraction operator. Subtracts two vectors.
	 *
	 * @param Other The vector to subtract.
	 *
	 * @return The result of the subtraction.
	 */
	OC_INLINE TVector4<T> operator-(const TVector4<T>& Other) const;

	/**
	 * Subtraction operator. Subtracts a vector from this.
	 *
	 * @param Other The vector to subtract.
	 *
	 * @return A reference to this, after the subtraction.
	 */
	OC_INLINE TVector4<T>& operator-=(const TVector4<T>& Other);

	/**
	 * Multiplication operator. Multiplies a scalar to this.
	 *
	 * @param Scalar The scalar to multiply.
	 *
	 * @return The result of the multiplication.
	 */
	OC_INLINE TVector4<T> operator*(T Scalar) const;

	/**
	 * Multiplication operator. Multiplies this with a scalar.
	 *
	 * @param Scalar The scalar to multiply.
	 *
	 * @return A reference to this, after the multiplication.
	 */
	OC_INLINE TVector4<T>& operator*=(T Scalar);
};

}

/**
*------------------------------------------------------------
* A 4-component vector with single-floating point precision.
*------------------------------------------------------------
* @see 'TVector4<T>'.
*/
using FVector4 = OC::TVector4<float>;

// TVector4 Declaration
#pragma endregion

namespace OC
{

#pragma region TVector2 Definition

template<typename T>
OC_INLINE TVector2<T>::TVector2()
	: X(T(0.0))
	, Y(T(0.0))
{}

template<typename T>
OC_INLINE TVector2<T>::TVector2(const TVector2<T>& Other)
	: X(Other.X)
	, Y(Other.Y)
{}

template<typename T>
OC_INLINE TVector2<T>::TVector2(T InX, T InY)
	: X(InX)
	, Y(InY)
{}

template<typename T>
OC_INLINE TVector2<T>::TVector2(T Scalar)
	: X(Scalar)
	, Y(Scalar)
{}

template<typename T>
OC_INLINE TVector2<T>::TVector2(const TVector3<T>& Vector3)
	: X(Vector3.X)
	, Y(Vector3.Y)
{}

template<typename T>
OC_INLINE TVector2<T>::TVector2(const TVector4<T>& Vector4)
	: X(Vector4.X)
	, Y(Vector4.Y)
{}

template<typename T>
OC_INLINE TVector2<T>& TVector2<T>::operator=(const TVector2<T>& Other)
{
	X = Other.X;
	Y = Other.Y;
	return *this;
}

template<typename T>
OC_INLINE TVector2<T> TVector2<T>::operator+(const TVector2<T>& Other) const
{
	return TVector2<T>(X + Other.X, Y + Other.Y);
}

template<typename T>
OC_INLINE TVector2<T>& TVector2<T>::operator+=(const TVector2<T>& Other)
{
	X += Other.X;
	Y += Other.Y;
	return *this;
}

template<typename T>
OC_INLINE TVector2<T> TVector2<T>::operator-(const TVector2<T>& Other) const
{
	return TVector2<T>(X - Other.X, Y - Other.Y);
}

template<typename T>
OC_INLINE TVector2<T>& TVector2<T>::operator-=(const TVector2<T>& Other)
{
	X -= Other.X;
	Y -= Other.Y;
	return *this;
}

template<typename T>
OC_INLINE TVector2<T> TVector2<T>::operator*(T Scalar) const
{
	return TVector2<T>(X * Scalar, Y * Scalar);
}

template<typename T>
OC_INLINE TVector2<T>& TVector2<T>::operator*=(T Scalar)
{
	X *= Scalar;
	Y *= Scalar;
	return *this;
}

template<typename T>
OC_INLINE TVector2<T> operator*(T Scalar, const TVector2<T>& Vector)
{
	return Vector * Scalar;
}

template<typename T>
OC_INLINE T TVector2<T>::LengthSquared() const
{
	return (X * X) + (Y * Y);
}

template<typename T>
OC_INLINE T TVector2<T>::Length() const
{
	return FMath::Sqrt(LengthSquared());
}

template<typename T>
OC_INLINE T TVector2<T>::DotProduct(const TVector2<T>& A, TVector2<T>& B)
{
	return (A.X * B.X) + (A.Y * B.Y);
}

template<typename T>
OC_INLINE T TVector2<T>::Dot(const TVector2<T>& Other) const
{
	return DotProduct(*this, Other);
}

template<typename T>
OC_INLINE T TVector2<T>::operator|(const TVector2<T>& Other) const
{
	return Dot(Other);
}

template<typename T>
OC_INLINE bool TVector2<T>::IsNormalized(T Tolerance) const
{
	return FMath::Abs(LengthSquared() - T(1.0)) <= Tolerance;
}

template<typename T>
OC_INLINE TVector2<T> TVector2<T>::GetNormal() const
{
	return (*this) * (T(1.0) / Length());
}

template<typename T>
OC_INLINE TVector2<T> TVector2<T>::GetNormalIf(T Tolerance) const
{
	T SquaredLength = LengthSquared();

	if (FMath::Abs(SquaredLength - T(1.0)) > Tolerance)
	{
		return (*this) * (T(1.0) / FMath::Sqrt(SquaredLength));
	}

	return (*this);
}

template<typename T>
OC_INLINE TVector2<T> TVector2<T>::GetSafeNormal(T Threshold, const TVector2<T>& ResultIfError) const
{
	T SquaredLength = LengthSquared();

	if (SquaredLength >= Threshold)
	{
		return (*this) * (T(1.0) / FMath::Sqrt(SquaredLength));
	}

	return ResultIfError;
}

template<typename T>
OC_INLINE TVector2<T> TVector2<T>::GetSafeNormalIf(T Threshold, const TVector2<T>& ResultIfError, T Tolerance) const
{
	T SquaredLength = LengthSquared();

	if (FMath::Abs(SquaredLength - T(1.0)) > Tolerance)
	{
		if (SquaredLength >= Threshold)
		{
			return (*this) * (T(1.0) / FMath::Sqrt(SquaredLength));
		}

		return ResultIfError;
	}

	return (*this);
}

// TVector2 Definition
#pragma endregion

#pragma region TVector3 Definition

template<typename T>
OC_INLINE TVector3<T>::TVector3()
	: X(T(0.0))
	, Y(T(0.0))
	, Z(T(0.0))
{}

template<typename T>
OC_INLINE TVector3<T>::TVector3(const TVector3& Other)
	: X(Other.X)
	, Y(Other.Y)
	, Z(Other.Z)
{}

template<typename T>
OC_INLINE TVector3<T>::TVector3(T InX, T InY, T InZ)
	: X(InX)
	, Y(InY)
	, Z(InZ)
{}

template<typename T>
OC_INLINE TVector3<T>::TVector3(T Scalar)
	: X(Scalar)
	, Y(Scalar)
	, Z(Scalar)
{}

template<typename T>
OC_INLINE TVector3<T>::TVector3(const TVector2<T>& Vector2)
	: X(Vector2.X)
	, Y(Vector2.Y)
	, Z(T(0.0))
{}

template<typename T>
OC_INLINE TVector3<T>::TVector3(const TVector2<T>& Vector2, T InZ)
	: X(Vector2.X)
	, Y(Vector2.Y)
	, Z(InZ)
{}

template<typename T>
OC_INLINE TVector3<T>::TVector3(const TVector4<T>& Vector4)
	: X(Vector4.X)
	, Y(Vector4.Y)
	, Z(Vector4.Z)
{}

template<typename T>
OC_INLINE TVector3<T>& TVector3<T>::operator=(const TVector3<T>& Other)
{
	X = Other.X;
	Y = Other.Y;
	Z = Other.Z;
	return *this;
}

template<typename T>
OC_INLINE TVector3<T> TVector3<T>::operator+(const TVector3<T>& Other) const
{
	return TVector3<T>(X + Other.X, Y + Other.Y, Z + Other.Z);
}

template<typename T>
OC_INLINE TVector3<T>& TVector3<T>::operator+=(const TVector3<T>& Other)
{
	X += Other.X;
	Y += Other.Y;
	Z += Other.Z;
	return *this;
}

template<typename T>
OC_INLINE TVector3<T> TVector3<T>::operator-(const TVector3<T>& Other) const
{
	return TVector3<T>(X - Other.X, Y - Other.Y, Z - Other.Z);
}

template<typename T>
OC_INLINE TVector3<T>& TVector3<T>::operator-=(const TVector3<T>& Other)
{
	X -= Other.X;
	Y -= Other.Y;
	Z -= Other.Z;
	return *this;
}

template<typename T>
OC_INLINE TVector3<T> TVector3<T>::operator*(T Scalar) const
{
	return TVector3<T>(X * Scalar, Y * Scalar, Z * Scalar);
}

template<typename T>
OC_INLINE TVector3<T>& TVector3<T>::operator*=(T Scalar)
{
	X *= Scalar;
	Y *= Scalar;
	Z *= Scalar;
	return *this;
}

template<typename T>
OC_INLINE TVector3<T> operator*(T Scalar, const TVector3<T>& Vector)
{
	return Vector * Scalar;
}

template<typename T>
OC_INLINE T TVector3<T>::LengthSquared() const
{
	return (X * X) + (Y * Y) + (Z * Z);
}

template<typename T>
OC_INLINE T TVector3<T>::Length() const
{
	return FMath::Sqrt(LengthSquared());
}

template<typename T>
OC_INLINE T TVector3<T>::DotProduct(const TVector3<T>& A, const TVector3<T>& B)
{
	return (A.X * B.X) + (A.Y * B.Y) + (A.Z * B.Z);
}

template<typename T>
OC_INLINE T TVector3<T>::Dot(const TVector3<T>& Other) const
{
	return DotProduct(*this, Other);
}

template<typename T>
OC_INLINE T TVector3<T>::operator|(const TVector3<T>& Other) const
{
	return Dot(Other);
}

template<typename T>
OC_INLINE TVector3<T> TVector3<T>::CrossProduct(const TVector3<T>& A, const TVector3<T>& B)
{
	return TVector3<T>
		(
			A.Y * B.Z - A.Z * B.Y,
			A.Z * B.X - A.X * B.Z,
			A.X * B.Y - A.Y * B.X
			);
}

template<typename T>
OC_INLINE TVector3<T> TVector3<T>::Cross(const TVector3<T>& Other) const
{
	return CrossProduct(*this, Other);
}

template<typename T>
OC_INLINE TVector3<T> TVector3<T>::operator^(const TVector3<T>& Other) const
{
	return Cross(Other);
}

template<typename T>
OC_INLINE TVector3<T>& TVector3<T>::operator^=(const TVector3<T>& Other)
{
	(*this) = (*this) ^ Other;
	return *this;
}

template<typename T>
OC_INLINE bool TVector3<T>::IsNormalized(T Tolerance) const
{
	return FMath::Abs(LengthSquared() - T(1.0)) <= Tolerance;
}

template<typename T>
OC_INLINE TVector3<T> TVector3<T>::GetNormal() const
{
	return (*this) * (T(1.0) / Length());
}

template<typename T>
OC_INLINE TVector3<T> TVector3<T>::GetNormalIf(T Tolerance) const
{
	T SquaredLength = LengthSquared();

	if (FMath::Abs(SquaredLength - T(1.0)) > Tolerance)
	{
		return (*this) * (T(1.0) / FMath::Sqrt(SquaredLength));
	}

	return (*this);
}

template<typename T>
OC_INLINE TVector3<T> TVector3<T>::GetSafeNormal(T Threshold, const TVector3<T>& ResultIfError) const
{
	T SquaredLength = LengthSquared();

	if (SquaredLength >= Threshold)
	{
		return (*this) * (T(1.0) / FMath::Sqrt(SquaredLength));
	}

	return ResultIfError;
}

template<typename T>
OC_INLINE TVector3<T> TVector3<T>::GetSafeNormalIf(T Threshold, const TVector3<T>& ResultIfError, T Tolerance) const
{
	T SquaredLength = LengthSquared();

	if (FMath::Abs(SquaredLength - T(1.0)) > Tolerance)
	{
		if (SquaredLength >= Threshold)
		{
			return (*this) * (T(1.0) / FMath::Sqrt(SquaredLength));
		}

		return ResultIfError;
	}

	return (*this);
}

// TVector3 Definition
#pragma endregion

#pragma region TVector4 Definition

template<typename T>
OC_INLINE TVector4<T>::TVector4()
	: X(T(0.0))
	, Y(T(0.0))
	, Z(T(0.0))
	, W(T(0.0))
{}

template<typename T>
OC_INLINE TVector4<T>::TVector4(const TVector4<T>& Other)
	: X(Other.X)
	, Y(Other.Y)
	, Z(Other.Z)
	, W(Other.W)
{}

template<typename T>
OC_INLINE TVector4<T>::TVector4(T InX, T InY, T InZ, T InW)
	: X(InX)
	, Y(InY)
	, Z(InZ)
	, W(InW)
{}

template<typename T>
OC_INLINE TVector4<T>::TVector4(T Scalar)
	: X(Scalar)
	, Y(Scalar)
	, Z(Scalar)
	, W(Scalar)
{}

template<typename T>
OC_INLINE TVector4<T>::TVector4(const TVector2<T>& Vector2)
	: X(Vector2.X)
	, Y(Vector2.Y)
	, Z(T(0.0))
	, W(T(0.0))
{}

template<typename T>
OC_INLINE TVector4<T>::TVector4(const TVector2<T>& Vector2, T InZ, T InW)
	: X(Vector2.X)
	, Y(Vector2.Y)
	, Z(InZ)
	, W(InW)
{}

template<typename T>
OC_INLINE TVector4<T>::TVector4(const TVector3<T>& Vector3)
	: X(Vector3.X)
	, Y(Vector3.Y)
	, Z(Vector3.Z)
	, W(T(0.0))
{}

template<typename T>
OC_INLINE TVector4<T>::TVector4(const TVector3<T>& Vector3, T InW)
	: X(Vector3.X)
	, Y(Vector3.Y)
	, Z(Vector3.Z)
	, W(InW)
{}

template<typename T>
OC_INLINE TVector4<T>& TVector4<T>::operator=(const TVector4<T>& Other)
{
	X = Other.X;
	Y = Other.Y;
	Z = Other.Z;
	W = Other.W;
	return *this;
}

template<typename T>
OC_INLINE TVector4<T> TVector4<T>::operator+(const TVector4<T>& Other) const
{
	return TVector4<T>(X + Other.X, Y + Other.Y, Z + Other.Z, W + Other.W);
}

template<typename T>
OC_INLINE TVector4<T>& TVector4<T>::operator+=(const TVector4<T>& Other)
{
	X += Other.X;
	Y += Other.Y;
	Z += Other.Z;
	W += Other.W;
	return *this;
}

template<typename T>
OC_INLINE TVector4<T> TVector4<T>::operator-(const TVector4<T>& Other) const
{
	return TVector4<T>(X - Other.X, Y - Other.Y, Z - Other.Z, W - Other.W);
}

template<typename T>
OC_INLINE TVector4<T>& TVector4<T>::operator-=(const TVector4<T>& Other)
{
	X -= Other.X;
	Y -= Other.Y;
	Z -= Other.Z;
	W -= Other.W;
	return *this;
}

template<typename T>
OC_INLINE TVector4<T> TVector4<T>::operator*(T Scalar) const
{
	return TVector4<T>(X * Scalar, Y * Scalar, Z * Scalar, W * Scalar);
}

template<typename T>
OC_INLINE TVector4<T>& TVector4<T>::operator*=(T Scalar)
{
	X *= Scalar;
	Y *= Scalar;
	Z *= Scalar;
	W *= Scalar;
	return *this;
}

template<typename T>
OC_INLINE TVector4<T> operator*(T Scalar, const TVector4<T>& Vector)
{
	return Vector * Scalar;
}

// TVector4 Definition
#pragma endregion

} // namespace OC

#if OC_IMPLEMENTATION

#include <cstdlib>
#include <cmath>
#include <cstdio>

#if OC_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // OC_PLATFORM_WINDOWS

namespace OC
{

void OnCheckFailed(const char* Expression, const char* File, const char* Function, uint32 Line, const char* Message)
{
	constexpr SizeT BufferSize = 512;

	const char Title[] = "CHECK FAILED";
	SizeT TitleLength = sizeof(Title) / sizeof(char) - 1;

	static char ExprString[BufferSize] = {};
	SizeT ExprStringLength = (SizeT)sprintf_s(ExprString, "EXPRESSION: %s", Expression);

	static char MesgString[BufferSize] = {};
	SizeT MesgStringLength = 0;
	if (Message)
	{
		MesgStringLength = (SizeT)sprintf_s(MesgString, "MESSAGE:    %s", Message);
	}

	static char FileString[BufferSize] = {};
	SizeT FileStringLength = (SizeT)sprintf_s(FileString, "FILE:       %s", File);

	static char FuncString[BufferSize] = {};
	SizeT FuncStringLength = (SizeT)sprintf_s(FuncString, "FUNCTION:   %s", Function);

	static char LineString[BufferSize] = {};
	SizeT LineStringLength = (SizeT)sprintf_s(LineString, "LINE:       %u", Line);

	SizeT MaxWidth = TitleLength + 2;
	MaxWidth = FMath::Max(MaxWidth, ExprStringLength);
	MaxWidth = FMath::Max(MaxWidth, MesgStringLength);
	MaxWidth = FMath::Max(MaxWidth, FileStringLength);
	MaxWidth = FMath::Max(MaxWidth, FuncStringLength);
	MaxWidth = FMath::Max(MaxWidth, LineStringLength);

	{
		SizeT HeaderLength = MaxWidth - (TitleLength + 2);
		SizeT HeaderPreOffset  = HeaderLength % 2;
		SizeT HeaderPostOffset = 0;

		char HeaderBuffer[BufferSize] = {};
		FMemory::Set(HeaderBuffer, '-', (HeaderLength / 2 + HeaderLength % 2) * sizeof(char));

		OC_CORE_FATAL("+-%s %s %s-+", HeaderBuffer + HeaderPreOffset, Title, HeaderBuffer + HeaderPostOffset);
	}
	{
		char WhitespaceBuffer[BufferSize] = {};
		FMemory::Set(WhitespaceBuffer, ' ', (MaxWidth - ExprStringLength) * sizeof(char));
		OC_CORE_FATAL("| %s%s |", ExprString, WhitespaceBuffer);
	}

	if (Message)
	{
		char WhitespaceBuffer[BufferSize] = {};
		FMemory::Set(WhitespaceBuffer, ' ', (MaxWidth - MesgStringLength) * sizeof(char));
		OC_CORE_FATAL("| %s%s |", MesgString, WhitespaceBuffer);
	}

	{
		char WhitespaceBuffer[BufferSize] = {};
		FMemory::Set(WhitespaceBuffer, ' ', (MaxWidth - FileStringLength) * sizeof(char));
		OC_CORE_FATAL("| %s%s |", FileString, WhitespaceBuffer);
	}
	{
		char WhitespaceBuffer[BufferSize] = {};
		FMemory::Set(WhitespaceBuffer, ' ', (MaxWidth - FuncStringLength) * sizeof(char));
		OC_CORE_FATAL("| %s%s |", FuncString, WhitespaceBuffer);
	}
	{
		char WhitespaceBuffer[BufferSize] = {};
		FMemory::Set(WhitespaceBuffer, ' ', (MaxWidth - LineStringLength) * sizeof(char));
		OC_CORE_FATAL("| %s%s |", LineString, WhitespaceBuffer);
	}
	{
		char FooterBuffer[BufferSize] = {};
		FMemory::Set(FooterBuffer, '-', MaxWidth * sizeof(char));
		OC_CORE_FATAL("+-%s-+\n", FooterBuffer);
	}
}

} // namespace OC

#pragma region Platform Windows Implementation

namespace OC
{

struct FWindowsPlatformData
{
#if OC_PLATFORM_WINDOWS
	HANDLE ConsoleHandle = INVALID_HANDLE_VALUE;

	// These have dummy initialization values.
	FPlatform::EConsoleColor ConsoleTextColor       = FPlatform::EConsoleColor::Purple;
	FPlatform::EConsoleColor ConsoleBackgroundColor = FPlatform::EConsoleColor::Purple;
#endif // OC_PLATFORM_WINDOWS
};
static FWindowsPlatformData* PlatformData = nullptr;

bool FPlatform::Initialize()
{
	if (PlatformData)
	{
		return false;
	}

#if OC_PLATFORM_WINDOWS
	PlatformData = (FWindowsPlatformData*)malloc(sizeof(FWindowsPlatformData));
	new (PlatformData) FWindowsPlatformData();

	PlatformData->ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleColor(EConsoleColor::White, EConsoleColor::Black);

	return true;
#endif // OC_PLATFORM_WINDOWS
}

void FPlatform::Shutdown()
{
	if (!PlatformData)
	{
		return;
	}

#if OC_PLATFORM_WINDOWS
	SetConsoleColor(EConsoleColor::White, EConsoleColor::Black);
#endif // OC_PLATFORM_WINDOWS

	(*PlatformData).~FWindowsPlatformData();
	free(PlatformData);
	PlatformData = nullptr;
}

void* FPlatform::AllocMemory(SizeT BlockSize)
{
#if OC_PLATFORM_WINDOWS
	return malloc((size_t)BlockSize);
#endif // OC_PLATFORM_WINDOWS
}

void FPlatform::FreeMemory(void* MemoryBlock)
{
#if OC_PLATFORM_WINDOWS
	free(MemoryBlock);
#endif // OC_PLATFORM_WINDOWS
}

bool FPlatform::SetConsoleColor(EConsoleColor TextColor, EConsoleColor BackgroundColor)
{
#if OC_PLATFORM_WINDOWS
	if (!PlatformData || PlatformData->ConsoleHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	if (TextColor == PlatformData->ConsoleTextColor && BackgroundColor == PlatformData->ConsoleBackgroundColor)
	{
		return true;
	}

	WORD Attributes = (WORD)(TextColor) | ((WORD)(BackgroundColor) << 4);
	BOOL Result = SetConsoleTextAttribute(PlatformData->ConsoleHandle, Attributes);
	return true;
#endif // OC_PLATFORM_WINDOWS
}

bool FPlatform::WriteToConsole(const char* Message, SizeT MessageSize)
{
#if OC_PLATFORM_WINDOWS
	if (!PlatformData)
	{
		// The platform was not initialized.
		return false;
	}

	BOOL Result = WriteConsoleA(PlatformData->ConsoleHandle, Message, (DWORD)MessageSize, NULL, NULL);
	return true;
#endif // OC_PLATFORM_WINDOWS
}

} // namespace OC

// Platform Windows Implementation
#pragma endregion

#pragma region Memory Implementation

bool FMemory::Initialize()
{
	return true;
}

void FMemory::Shutdown()
{

}

OC_NODISCARD void* FMemory::AllocateRaw(SizeT BlockSize)
{
	if (BlockSize == 0)
	{
		return nullptr;
	}
	return OC::FPlatform::AllocMemory(BlockSize);
}

OC_NODISCARD void* FMemory::Allocate(SizeT BlockSize)
{
	if (BlockSize == 0)
	{
		return nullptr;
	}

	return FMemory::AllocateRaw(BlockSize);
}

OC_NODISCARD void* FMemory::AllocateTagged(SizeT BlockSize)
{
	if (BlockSize == 0)
	{
		return nullptr;
	}

	return FMemory::AllocateRaw(BlockSize);
}

void FMemory::FreeRaw(void* MemoryBlock)
{
	OC::FPlatform::FreeMemory(MemoryBlock);
}

void FMemory::Free(void* MemoryBlock)
{
	if (MemoryBlock == nullptr)
	{
		return;
	}

	FMemory::FreeRaw(MemoryBlock);
}

void FMemory::Copy(void* Destination, const void* Source, SizeT Size)
{
	memcpy(Destination, Source, (size_t)Size);
}

void FMemory::Set(void* Destination, uint8 Value, SizeT Size)
{
	memset(Destination, (int)Value, (size_t)Size);
}

void FMemory::Zero(void* Destination, SizeT Size)
{
	FMemory::Set(Destination, 0, Size);
}

// Memory Implementation
#pragma endregion

#pragma region Logger Implementation

namespace OC
{

constexpr FPlatform::EConsoleColor ConsoleColors[2 * 6] =
{
	/**           Text Color                         Background Color            */
	FPlatform::EConsoleColor::Purple,     FPlatform::EConsoleColor::Black, // Debug
	FPlatform::EConsoleColor::Gray,       FPlatform::EConsoleColor::Black, // Trace
	FPlatform::EConsoleColor::Green,      FPlatform::EConsoleColor::Black, // Info
	FPlatform::EConsoleColor::LightRed,   FPlatform::EConsoleColor::Black, // Error
	FPlatform::EConsoleColor::Yellow,     FPlatform::EConsoleColor::Black, // Warn
	FPlatform::EConsoleColor::LightWhite, FPlatform::EConsoleColor::Red    // Fatal
};

constexpr const char* LogTypesNames[6] =
{
	"DEBUG", "TRACE", "INFO", "WARN", "ERROR", "FATAL"
};

void FLogger::Write(const char* Tag, ELogType LogType, const char* Message, ...)
{
	auto TextColor       = ConsoleColors[2 * (uint8)LogType + 0];
	auto BackgroundColor = ConsoleColors[2 * (uint8)LogType + 1];

	static char buffer[8192] = {};
	FMemory::Zero(buffer, sizeof(buffer));

	va_list vl;
	va_start(vl, Message);
	vsprintf_s(buffer, Message, vl);
	va_end(vl);

	static char buffer2[8192] = {};
	FMemory::Zero(buffer2, sizeof(buffer2));
	int Written = sprintf_s(buffer2, "[%s][%s]: %s\n", LogTypesNames[(uint8)LogType], Tag, buffer);

	FPlatform::SetConsoleColor(TextColor, BackgroundColor);
	FPlatform::WriteToConsole(buffer2, (SizeT)Written);
}

} // namespace OC

// Logger Implementation
#pragma endregion

#pragma region Math Utilities Implementation

float FMath::Sqrt(float X)
{
	return sqrtf(X);
}

double FMath::Sqrt(double X)
{
	return sqrt(X);
}

// Math Utilities Implementation
#pragma endregion

#pragma region Application Entry Point

bool OnCreate();

void OnUpdate(float DeltaTime);

void OnDestroy();

namespace OC
{

static bool InitializeCore()
{
	if (!FPlatform::Initialize())
	{
		return false;
	}

	if (!FMemory::Initialize())
	{
		FPlatform::Shutdown();
		return false;
	}

	return true;
}

static void ShutdownCore()
{
	FMemory::Shutdown();
	FPlatform::Shutdown();
}

static int32 Main(char** ArgValues, uint32 ArgsCount)
{
	// Initialize the core systems.
	Verify(InitializeCore());

	if (!OnCreate())
	{
		return -1;
	}

	bool IsRunning = true;
	while (IsRunning)
	{
		float DeltaTime = 0.0f;
		OnUpdate(DeltaTime);
	}
	IsRunning = false;

	OnDestroy();

	// Shutdown the core systems.
	ShutdownCore();
	return 0;
}

} // namespace OC

int main(int ArgsCount, char** ArgValues)
{
	return OC::Main(ArgValues, ArgsCount);
}

// Application Entry Point
#pragma endregion

#endif // OC_IMPLEMENTATION