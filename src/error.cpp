/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "error.hpp"

namespace Invader {
    const char *InvalidScenarioNameException::what() const noexcept {
        return "scenario name is invalid";
    }

    const char *InvalidPointerException::what() const noexcept {
        return "pointer is invalid";
    }

    const char *InvalidDependencyException::what() const noexcept {
        return "dependency is invalid";
    }

    const char *MaximumTagDataSizeException::what() const noexcept {
        return "maximum tag data size exceeded";
    }

    const char *MaximumFileSizeException::what() const noexcept {
        return "maximum file size exceeded";
    }

    const char *OutOfBoundsException::what() const noexcept {
        return "data is out of bounds";
    }

    const char *InvalidCyclicLoopException::what() const noexcept {
        return "invalid cyclic loop";
    }

    const char *ExtraTagDataException::what() const noexcept {
        return "expected EOF found extra tag data";
    }

    const char *UnexpectedTagClassException::what() const noexcept {
        return "unexpected tag class";
    }

    const char *UnknownTagClassException::what() const noexcept {
        return "unknown tag class";
    }

    const char *FailedToOpenTagException::what() const noexcept {
        return "failed to open a tag";
    }

    const char *TagErrorException::what() const noexcept {
        return "an error occured with a tag";
    }

    const char *InvalidMapException::what() const noexcept {
        return "tried to open an invalid map";
    }

    const char *InvalidTagPathException::what() const noexcept {
        return "tag path is too long";
    }
}
