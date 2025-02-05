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

#ifndef PHENIQS_READ_H
#define PHENIQS_READ_H

#include "include.h"
#include "auxiliary.h"

/*
    typedef struct {
        int32_t     tid;            chromosome ID, defined by bam_hdr_t
        int32_t     pos;            0-based leftmost coordinate
        uint16_t    bin;            bin calculated by bam_reg2bin()
        uint8_t     qual;           mapping quality
        uint8_t     l_qname;        length of the query name
        uint16_t    flag;           bitwise flag
        uint8_t     unused1;
        uint8_t     l_extranul;     length of extra NULs between qname & cigar (for alignment)
        uint32_t    n_cigar;        number of CIGAR operations
        int32_t     l_qseq;         length of the query sequence (read)
        int32_t     mtid;           chromosome ID of next read in template, defined by bam_hdr_t
        int32_t     mpos;           0-based leftmost coordinate of next read in template
        int32_t     isize;
    } bam1_core_t;

    typedef struct {
        bam1_core_t core;
        int         l_data;
        uint32_t    m_data;
        uint8_t*    data;
        uint64_t    id;
    } bam1_t;
*/
class Segment : public ObservedSequence {
    friend ostream& operator<<(ostream& o, const Segment& segment);

    public:
        void operator=(Segment const &) = delete;
        Segment(Segment const &) = delete;
        size_t index;
        Platform platform;
        kstring_t name;
        uint16_t flag;
        Auxiliary auxiliary;

        #if defined(PHENIQS_SAM_ALIGNMENT)
        int32_t     tid;
        int32_t     pos;
        uint16_t    bin;
        uint8_t     qual;
        uint32_t    n_cigar;
        int32_t     mtid;
        int32_t     mpos;
        #endif

        inline void clear() override {
            ObservedSequence::clear();
            ks_clear(name);
            set_qcfail(false);
            auxiliary.clear();
        };
        inline uint32_t segment_index() const {
            if(!auxiliary.FI) {
                if(flag & uint16_t(HtsFlag::PAIRED)) {
                    if(flag & uint16_t(HtsFlag::READ1)) {
                        return 1;
                    } else if(flag & uint16_t(HtsFlag::READ2)) {
                        return 2;
                    } else {
                        throw SequenceError("inconsistent SAM flags");
                    }
                } else {
                    return 1;
                }
            } else {
                return auxiliary.FI;
            }
        };
        inline uint32_t total_segments() const {
            if(!auxiliary.TC) {
                if(flag & uint16_t(HtsFlag::PAIRED)) {
                    return 2;
                } else {
                    return 1;
                }
            } else {
                return auxiliary.TC;
            }
        };
        inline bool paired() const {
            return flag & uint16_t(HtsFlag::PAIRED);
        };
        inline void set_paired(const bool value) {
            if(value) {
                flag |= uint16_t(HtsFlag::PAIRED);
            } else {
                flag &= ~uint16_t(HtsFlag::PAIRED);
            }
        };
        inline bool qcfail() const {
            return flag & uint16_t(HtsFlag::QCFAIL);
        };
        inline void set_qcfail(const bool value) {
            if(value) {
                flag |= uint16_t(HtsFlag::QCFAIL);
            } else {
                flag &= ~uint16_t(HtsFlag::QCFAIL);
            }
        };
        inline bool first_segment() const {
            return flag & uint16_t(HtsFlag::READ1);
        };
        inline void set_first_segment(const bool value) {
            if(value) {
                flag |= uint16_t(HtsFlag::READ1);
            } else {
                flag &= ~uint16_t(HtsFlag::READ1);
            }
        };
        inline bool last_segment() const {
            return flag & uint16_t(HtsFlag::READ2);
        };
        inline void set_last_segment(const bool value) {
            if(value) {
                flag |= uint16_t(HtsFlag::READ2);
            } else {
                flag &= ~uint16_t(HtsFlag::READ2);
            }
        };
        Segment() :
            ObservedSequence(),
            index(0),
            platform(Platform::UNKNOWN),
            name({ 0, 0, NULL }),
            flag(0),
            auxiliary() {

            ks_terminate(name);
            flag |= uint16_t(HtsFlag::UNMAP);
            flag |= uint16_t(HtsFlag::MUNMAP);
        };
        ~Segment() override {
            ks_free(name);
        };
};
ostream& operator<<(ostream& o, const Segment& segment);

class Read : public SequenceArray< Segment > {
    friend ostream& operator<<(ostream& o, const Read& read);

    protected:
        Segment* leader;

    public:
        void operator=(Read const &) = delete;
        Read(Read const &) = delete;
        const Platform platform;
        uint32_t multiplex_distance;
        double multiplex_decoding_confidence;
        uint32_t molecular_distance;
        double molecular_decoding_confidence;
        uint32_t cellular_distance;
        double cellular_decoding_confidence;
        double barcode_decoding_confidence;

        inline void clear() override {
            for(auto& segment : segment_array) {
                segment.clear();
            }
            multiplex_distance = 0;
            multiplex_decoding_confidence = 1;
            molecular_distance = 0;
            molecular_decoding_confidence = 1;
            cellular_distance = 0;
            cellular_decoding_confidence = 1;
            barcode_decoding_confidence = 1;
        };
        inline void flush() {
            if(multiplex_decoding_confidence > 0 && multiplex_decoding_confidence < 1) {
                leader->auxiliary.XB = static_cast< float >(1.0 - multiplex_decoding_confidence);
                // barcode_decoding_confidence *= multiplex_decoding_confidence;
            }
            if(molecular_decoding_confidence > 0 && molecular_decoding_confidence < 1) {
                leader->auxiliary.XM = static_cast< float >(1.0 - molecular_decoding_confidence);
                // barcode_decoding_confidence *= molecular_decoding_confidence;
            }
            if(cellular_decoding_confidence > 0 && cellular_decoding_confidence < 1) {
                leader->auxiliary.XC = static_cast< float >(1.0 - cellular_decoding_confidence);
                // barcode_decoding_confidence *= cellular_decoding_confidence;
            }
            // if(barcode_decoding_confidence > 0 && barcode_decoding_confidence < 1) {
            //     leader->auxiliary.XO = static_cast< float >(1 - barcode_decoding_confidence);
            // }
            if(segment_cardinality() > 1) {
                for(auto& segment : this->segment_array) {
                    if(leader != &segment) {
                        segment.set_qcfail(leader->qcfail());
                        if(ks_not_empty(leader->auxiliary.RG)) ks_put_string(leader->auxiliary.RG, segment.auxiliary.RG);
                        if(ks_not_empty(leader->auxiliary.BC)) ks_put_string(leader->auxiliary.BC, segment.auxiliary.BC);
                        if(ks_not_empty(leader->auxiliary.QT)) ks_put_string(leader->auxiliary.QT, segment.auxiliary.QT);
                        if(leader->auxiliary.XB > 0)           segment.auxiliary.XB = leader->auxiliary.XB;

                        if(ks_not_empty(leader->auxiliary.RX)) ks_put_string(leader->auxiliary.RX, segment.auxiliary.RX);
                        if(ks_not_empty(leader->auxiliary.QX)) ks_put_string(leader->auxiliary.QX, segment.auxiliary.QX);
                        if(ks_not_empty(leader->auxiliary.OX)) ks_put_string(leader->auxiliary.OX, segment.auxiliary.OX);
                        if(ks_not_empty(leader->auxiliary.BZ)) ks_put_string(leader->auxiliary.BZ, segment.auxiliary.BZ);
                        if(ks_not_empty(leader->auxiliary.MI)) ks_put_string(leader->auxiliary.MI, segment.auxiliary.MI);
                        if(leader->auxiliary.XM > 0)           segment.auxiliary.XM = leader->auxiliary.XM;

                        if(ks_not_empty(leader->auxiliary.CB)) ks_put_string(leader->auxiliary.CB, segment.auxiliary.CB);
                        if(ks_not_empty(leader->auxiliary.CR)) ks_put_string(leader->auxiliary.CR, segment.auxiliary.CR);
                        if(ks_not_empty(leader->auxiliary.CY)) ks_put_string(leader->auxiliary.CY, segment.auxiliary.CY);
                        if(leader->auxiliary.XC > 0)           segment.auxiliary.XC = leader->auxiliary.XC;
                        if(leader->auxiliary.XO > 0)           segment.auxiliary.XO = leader->auxiliary.XO;
                    }
                }
            }
        };
        inline const Auxiliary& auxiliary() const {
            return leader->auxiliary;
        };
        inline const kstring_t& name() const {
            return leader->name;
        };
        inline const bool qcfail() const {
            return leader->qcfail();
        };
        inline void set_qcfail(const bool value) {
            return leader->set_qcfail(value);
        };
        inline const kstring_t& RG() const {
            return leader->auxiliary.RG;
        };
        inline void validate() const {
            if(segment_array.size() > 1) {
                /* validate that all segments in the read have the same identifier */
                const kstring_t& baseline = segment_array.front().name;
                for(size_t i(1); i < segment_array.size(); ++i) {
                    const Segment& segment = segment_array[i];
                    if((baseline.l != segment.name.l) || strncmp(baseline.s, segment.name.s, baseline.l)) {
                        throw SequenceError("read out of sync " + string(segment.name.s, segment.name.l) + " and " + string(baseline.s, baseline.l));
                    }
                }
            }
        };
        inline void assign_RG(const HeadRGAtom& rg) {
            leader->auxiliary.set_RG(rg);
        };

        inline void update_multiplex_barcode(const Observation& observation) {
            leader->auxiliary.update_multiplex_barcode(observation);
        };
        inline void update_multiplex_decoding_confidence(const double& confidence) {
            if(multiplex_decoding_confidence == 1) {
                multiplex_decoding_confidence = confidence;
            } else {
                multiplex_decoding_confidence *= confidence;
            }
        };
        inline void set_multiplex_decoding_confidence(const double& confidence) {
            multiplex_decoding_confidence = confidence;
        };
        inline void update_multiplex_distance(const uint32_t& distance) {
            multiplex_distance += distance;
        };
        inline void set_multiplex_distance(const uint32_t& distance) {
            multiplex_distance = distance;
        };

        inline void update_molecular_barcode(const Barcode& barcode) {
            leader->auxiliary.update_molecular_barcode(barcode);
        };
        inline void update_molecular_barcode(const Observation& observation) {
            leader->auxiliary.update_molecular_barcode(observation);
        };
        inline void update_molecular_decoding_confidence(const double& confidence) {
            if(molecular_decoding_confidence == 1) {
                molecular_decoding_confidence = confidence;
            } else {
                molecular_decoding_confidence *= confidence;
            }
        };
        inline void set_molecular_decoding_confidence(const double& confidence) {
            molecular_decoding_confidence = confidence;
        };
        inline void update_molecular_distance(const uint32_t& distance) {
            molecular_distance += distance;
        };
        inline void set_molecular_distance(const uint32_t& distance) {
            molecular_distance += distance;
        };
        inline void update_raw_molecular_barcode(const Observation& observation) {
            leader->auxiliary.update_raw_molecular_barcode(observation);
        };

        inline void update_cellular_barcode(const Barcode& barcode) {
            leader->auxiliary.update_cellular_barcode(barcode);
        };
        inline void update_cellular_barcode(const Observation& observation) {
            leader->auxiliary.update_cellular_barcode(observation);
        };
        inline void update_cellular_decoding_confidence(const double& confidence) {
            if(cellular_decoding_confidence == 1) {
                cellular_decoding_confidence = confidence;
            } else {
                cellular_decoding_confidence *= confidence;
            }
        };
        inline void set_cellular_decoding_confidence(const double& confidence) {
            cellular_decoding_confidence = confidence;
        };
        inline void update_cellular_distance(const uint32_t& distance) {
            cellular_distance += distance;
        };
        inline void set_cellular_distance(const uint32_t& distance) {
            cellular_distance = distance;
        };
        inline void update_raw_cellular_barcode(const Observation& observation) {
            leader->auxiliary.update_raw_cellular_barcode(observation);
        };

        Read(const int32_t& cardinality, const Platform& platform, int32_t leading_segment_index) :
            SequenceArray< Segment >(cardinality),
            leader(&segment_array[leading_segment_index]),
            platform(platform) {

            int32_t segment_index(0);
            for(auto& segment : segment_array) {
                segment.index = segment_index;
                segment.platform = platform;
                segment.auxiliary.FI = segment_index + 1;
                segment.auxiliary.TC = cardinality;
                if(cardinality > 1) {
                    segment.set_paired(true);
                }
                ++segment_index;
            }

            /* set first output segment READ1 flag ON */
            if(cardinality > 0) {
                segment_array.front().set_first_segment(true);
            }

            /* set last output segment READ2 flag ON */
            if(cardinality > 1) {
                segment_array.back().set_last_segment(true);
            }
        };
};
ostream& operator<<(ostream& o, const Read& read);

#endif /* PHENIQS_READ_H */
