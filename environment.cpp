/*
    Pheniqs : PHilology ENcoder wIth Quality Statistics
    Copyright (C) 2018  Lior Galanti
    NYU Center for Genetics and System Biology

    Author: Lior Galanti <lior.galanti@nyu.edu>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "environment.h"

Environment::Environment(const int argc, const char** argv) try :
    interface(argc, argv),
    _help_only(interface.help_triggered()),
    _version_only(interface.version_triggered()) {

    } catch(ConfigurationError& error) {
        throw ConfigurationError("Environment :: " + error.message);

    } catch(exception& error) {
        throw InternalError("Environment :: " + string(error.what()));
};
Environment::~Environment() {
    for(auto& job : job_queue) {
        delete job;
    }
};
void Environment::print_help(ostream& o) const {
    interface.print_help(o);
};
void Environment::print_version(ostream& o) const {
    interface.print_version(o);

    /*  This is mostly for when building a static pheniqs binary
        The --version will report the library versions that were built in. */

    #ifdef ZLIB_VERSION
    o << "zlib " << ZLIB_VERSION << endl;
    #endif

    #ifdef BZIP2_VERSION
    o << "bzlib " << BZIP2_VERSION << endl;
    #endif

    #ifdef XZ_VERSION
    o << "xzlib " << XZ_VERSION << endl;
    #endif

    #ifdef LIBDEFLATE_VERSION
    o << "libdeflate " << LIBDEFLATE_VERSION << endl;
    #endif

    #ifdef RAPIDJSON_VERSION
    o << "rapidjson " << RAPIDJSON_VERSION << endl;
    #endif

    #ifdef HTSLIB_VERSION
    o << "htslib " << HTSLIB_VERSION << endl;
    #endif
};
Job* Environment::pop_from_queue() {
    if(!job_queue.empty()) {
        Job* job(job_queue.front());
        job_queue.pop_front();
        return job;
    } else { return NULL;}
};
void Environment::push_to_queue(Document& operation) {
    if(operation.IsObject()) {
        Job* job(NULL);
        string implementation(decode_value_by_key< string >("implementation", operation));

        if(implementation == "multiplex") { job = new MultiplexJob(operation); }
        else { job = new Job(operation); }

        job->assemble();
        job_queue.emplace_back(job);
    } else { throw ConfigurationError("Job operation element is not a dictionary"); }
};
void Environment::execute_job(Job* job) {
    if(job != NULL) {
        job->compile();
        if(job->is_validate_only()) {
            job->describe(cerr);

        } else if(job->is_lint_only()) {
            job->print_ontology(cout);

        } else {
            job->execute();
            job->print_report(cerr);

        }
    }
};
void Environment::execute() {
    if(is_help_only()) {
        print_help(cerr);

    } else if(is_version_only()) {
        print_version(cerr);

    } else {
        Document operation(interface.operation());
        push_to_queue(operation);

        Job* job(NULL);
        while((job = pop_from_queue()) != NULL) {
            execute_job(job);
            delete job;
        }
    }
};
