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

#include "pamld.h"

template < class T > PhredAdjustedMaximumLikelihoodDecoder< T >::PhredAdjustedMaximumLikelihoodDecoder(const Value& ontology) try :
    ObservingDecoder< T >(ontology),
    noise(decode_value_by_key< double >("noise", ontology)),
    confidence_threshold(decode_value_by_key< double >("confidence threshold", ontology)),
    random_barcode_probability(1.0 / double(pow(4, (this->nucleotide_cardinality)))),
    adjusted_noise_probability(noise * random_barcode_probability),
    conditional_decoding_probability(0),
    decoding_confidence(0) {

    } catch(Error& error) {
        error.push("PhredAdjustedMaximumLikelihoodDecoder");
        throw;
};
template < class T > void PhredAdjustedMaximumLikelihoodDecoder< T >::classify(const Read& input, Read& output) {
    this->observation.clear();
    this->rule.apply(input, this->observation);

    /*  Compute the posterior probability P(observed|barcode) for each barcode
        Keep track of the channel that yield the maximal prior adjusted probability.
        If r is the observed sequence and b is the barcode sequence
        P(r|b) is the probability that r was observed given b was sequenced.
        Accumulate probabilities P(b) * P(r|b), in sigma_p using the Kahan summation algorithm
        to minimize floating point drift. see https://en.wikipedia.org/wiki/Kahan_summation_algorithm

        p is the prior adjusted conditional probability
        d is the decoding Hamming distance
        sigma_p accumulates the prior adjusted conditional probabilities to compute the decoding confidence
    */
    double y(0);
    double t(0);
    double p(0);
    int32_t d(0);
    double sigma_p(0);
    double compensation(0);
    double conditional_probability(0);
    double adjusted_conditional_decoding_probability(0);
    for(auto& barcode : this->element_by_index) {
        /*  conditional_probability: P(r|b) is the conditional_probability, the probability to observe the observation r given that b was sequenced
            barcode.concentration: P(b), the prior probability of observing b
            p: P(b) * P(r|b), the prior adjusted conditional probability
            sigma_p: the sum of p over b
        */
        barcode.compensated_decoding_probability(this->observation, conditional_probability, d);
        p = conditional_probability * barcode.concentration;
        y = p - compensation;
        t = sigma_p + y;
        compensation = (t - sigma_p) - y;
        sigma_p = t;
        if(p > adjusted_conditional_decoding_probability) {
            this->decoded = &barcode;
            this->decoding_hamming_distance = d;
            adjusted_conditional_decoding_probability = p;
            conditional_decoding_probability = conditional_probability;
        }
    }

    /*  Only proceed if the conditional_decoding_probability is higher than random_barcode_probability
        Note to self: it might be a better idea to compare to the probability of "any sequence not in the barcode set"
        which is 1 / (4^n - |B|). This will make the threshold more agressive, meaning reject more, as |B| gets bigger.
    */
    if(conditional_decoding_probability > random_barcode_probability) {

        /* add the prior adjusted noise probability to sigma_p */
        y = adjusted_noise_probability - compensation;
        t = sigma_p + y;
        compensation = (t - sigma_p) - y;
        sigma_p = t;

        /*  decoding_confidence: P(b|r) is the posterior probability, the probability that barcode b was sequenced given the observation r
            adjusted_conditional_decoding_probability: is the prior adjusted conditional probability of decoding the decoded barcode */
        decoding_confidence = adjusted_conditional_decoding_probability / sigma_p;

        /*  if the posterior probability is higher than the confidence_threshold */
        if(decoding_confidence > confidence_threshold) {
            this->decoded->accumulated_confidence += decoding_confidence;
            if(!input.qcfail()) {
                this->decoded->accumulated_pf_confidence += decoding_confidence;
            }

        } else {
            ++this->decoded->low_confidence_count;
            this->decoded = &this->unclassified;
            this->decoding_hamming_distance = 0;
            decoding_confidence = 0;
        }

    } else {
        ++this->decoded->low_conditional_confidence_count;
        this->decoded = &this->unclassified;
        this->decoding_hamming_distance = 0;
        decoding_confidence = 0;
    }
    ObservingDecoder< T >::classify(input, output);
};

PAMLMultiplexDecoder::PAMLMultiplexDecoder(const Value& ontology) try :
    PhredAdjustedMaximumLikelihoodDecoder< Channel >(ontology) {

    } catch(Error& error) {
        error.push("PAMLMultiplexDecoder");
        throw;
};
void PAMLMultiplexDecoder::classify(const Read& input, Read& output) {
    PhredAdjustedMaximumLikelihoodDecoder< Channel >::classify(input, output);
    output.assign_RG(this->decoded->rg);
    output.update_multiplex_barcode(this->observation);
    output.update_multiplex_distance(this->decoding_hamming_distance);
    output.update_multiplex_decoding_confidence(this->decoding_confidence);
};

PAMLCellularDecoder::PAMLCellularDecoder(const Value& ontology) try :
    PhredAdjustedMaximumLikelihoodDecoder< Barcode >(ontology) {

    } catch(Error& error) {
        error.push("PAMLCellularDecoder");
        throw;
};
void PAMLCellularDecoder::classify(const Read& input, Read& output) {
    PhredAdjustedMaximumLikelihoodDecoder< Barcode >::classify(input, output);
    output.update_raw_cellular_barcode(this->observation);
    output.update_cellular_barcode(*this->decoded);
    if(this->decoded->is_classified()) {
        output.update_cellular_decoding_confidence(this->decoding_confidence);
        output.update_cellular_distance(this->decoding_hamming_distance);
    } else {
        output.set_cellular_decoding_confidence(0);
        output.set_cellular_distance(0);
    }
};