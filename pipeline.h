/* Pheniqs : PHilology ENcoder wIth Quality Statistics
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

#ifndef PHENIQS_ENVIRONMENT_H
#define PHENIQS_ENVIRONMENT_H

#include "include.h"
#include "interface.h"
#include "transcode.h"

class Pipeline {
    public:
        Pipeline(const int argc, const char** argv);
        ~Pipeline();
        inline const bool is_help_only() const {
            return _help_only;
        };
        inline const bool is_version_only() const {
            return _version_only;
        };
        void execute();
        void print_help(ostream& o) const;
        void print_version(ostream& o) const;
        void push_to_queue(Document& operation);
        Job* pop_from_queue();

    private:
        const Interface interface;
        list< Job* > job_queue;
        const bool _help_only;
        const bool _version_only;
        void execute_job(Job* job);
};

#endif /* PHENIQS_ENVIRONMENT_H */
