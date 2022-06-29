#pragma once

#if __clang__

#pragma clang attribute push
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wsign-compare"

#elif __GNUC__

/*
 -Werror -Wall -Wextra -Wcast-align -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wduplicated-branches -Wduplicated-cond -Wextra-semi -Wfloat-equal -Wlogical-op -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wno-error=cpp -Wno-error=unused-parameter -Wno-error=switch -Wno-sign-conversion -Wno-old-style-cast -Wno-conversion -Wno-nonnull-compare -Wno-error=sign-promo -Wno-error=deprecated-copy -Wno-error=unused-variable
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"

#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#pragma GCC diagnostic ignored "-Wduplicated-cond"
#pragma GCC diagnostic ignored "-Wextra-semi"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wlogical-op"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-promo"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wattributes"


               

#elif _MSC_VER

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)
#pragma warning(disable : 4624)
#pragma warning(disable : 4067)
#pragma warning(disable : 4275)
#pragma warning(disable : 4101)
#pragma warning(disable : 4267)
#pragma warning(disable : 4015)
#pragma warning(disable : 4014)
#pragma warning(disable : 4067)
#pragma warning(disable : 4005)

#endif

