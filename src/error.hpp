/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__ERROR_HPP
#define INVADER__ERROR_HPP

#include <exception>

namespace Invader {
    /**
     * This is thrown when a map has an invalid name.
     */
    class InvalidScenarioNameException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when a pointer is invalid.
     */
    class InvalidPointerException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when a dependency is invalid.
     */
    class InvalidDependencyException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when tag data exceeds the maximum tag data size
     */
    class MaximumTagDataSizeException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when tag data exceeds the maximum file size
     */
    class MaximumFileSizeException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when tag data is out of bounds.
     */
    class OutOfBoundsException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when an invalid cyclic loop is present, likely resulting in infinite recursion.
     */
    class InvalidCyclicLoopException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when tag data remains
     */
    class ExtraTagDataException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when a tag cannot be compiled because its tag class is wrong
     */
    class UnexpectedTagClassException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when a tag cannot be compiled because it's unknown
     */
    class UnknownTagClassException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when a tag cannot be compiled because it's not found or could not be opened
     */
    class FailedToOpenTagException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when some other tag related error occurs.
     */
    class TagErrorException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when a map isn't valid and tried to be opened
     */
    class InvalidMapException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };

    /**
     * This is thrown when a tag path is too long
     */
    class InvalidTagPathException : public std::exception {
    public:
        /** What happened? */
        const char *what() const noexcept;
    };
}
#endif
