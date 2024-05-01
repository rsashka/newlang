#pragma once
#ifndef INCLUDED_MODULE_CPP_
#define INCLUDED_MODULE_CPP_

//#include "pch.h"
#include "nlc-rt.h"
#include "term.h"

//#include <types.h>
//#include <object.h>
#include "runtime.h"
#include "context.h"

namespace newlang {


    class Module : public std::map<std::string, VarItem> {
    public:
        RunTime *m_rt;
        
        std::string m_file;
        std::string m_md5;
        std::string m_timestamp;
        std::string m_version;
//        std::vector<llvm::orc::ThreadModule> m_llvm_module;
        //        std::string m_source;
        //        bool m_is_main;

        TermPtr m_ast;
    public:

        Module(RunTime *rt = nullptr, TermPtr ast = nullptr);
        static void RegisterStaticObject(Module &module, TermPtr term, bool init);

        //        bool Load(Context & ctx, const char * path, bool is_main) {
        //            m_is_main = is_main;
        //            m_file = path;
        //            //            m_var_name = ExtractModuleName(path);
        //            auto file = llvm::sys::fs::openNativeFileForRead(path);
        //            if (!file) {
        //                //                LOG_ERROR("Error open module '%s' from file %s!", m_var_name.c_str(), path);
        //                return false;
        //            }
        //
        //
        //            char buffer[llvm::sys::fs::DefaultReadChunkSize];
        //            llvm::MutableArrayRef<char> Buf(buffer, llvm::sys::fs::DefaultReadChunkSize);
        //
        //            llvm::Expected<size_t> readed(0);
        //            do {
        //                readed = llvm::sys::fs::readNativeFile(*file, Buf);
        //                if (!readed) {
        //                    //                    LOG_ERROR("Error read module '%s' from file %s!", m_var_name.c_str(), path);
        //                    return false;
        //                }
        //                m_source.append(Buf.data(), *readed);
        //            } while (*readed);
        //
        //            llvm::sys::fs::file_status fs;
        //            std::error_code ec = llvm::sys::fs::status(*file, fs);
        //            if (ec) {
        //                m_timestamp = "??? ??? ?? ??:??:?? ????";
        //            } else {
        //                //                auto tp = fs.getLastModificationTime();
        //                time_t temp = llvm::sys::toTimeT(fs.getLastModificationTime());
        //                struct tm * timeinfo;
        //                timeinfo = localtime(&temp);
        //                m_timestamp = asctime(timeinfo);
        //                m_timestamp = m_timestamp.substr(0, 24); // Remove \n on the end line
        //
        //            }
        //
        //            llvm::ErrorOr<llvm::MD5::MD5Result> md5 = llvm::sys::fs::md5_contents((int) *file);
        //            if (!md5) {
        //                m_md5 = "????????????????????????????????";
        //            } else {
        //                llvm::SmallString<32> hash;
        //                llvm::MD5::stringifyResult((*md5), hash);
        //                m_md5 = hash.c_str();
        //            }
        //            llvm::sys::fs::closeFile(*file);
        //
        //            //            m_var_is_init = true;
        //            return true;
        //        }

        virtual ~Module() {

        }

        std::string Dump() {
            std::string result;

            for (auto &elem : * this) {
                result += '\n';
                result += elem.first;
            }

            return result;
        }

    };


}


#endif //INCLUDED_MODULE_CPP_
