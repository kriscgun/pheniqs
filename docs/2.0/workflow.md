<!--
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
-->

<section id="navigation">
    <ul>
        <li><a                  href="/pheniqs/2.0/">Home</a></li>
        <li><a                  href="/pheniqs/2.0/tutorial.html">Tutorial</a></li>
        <li><a                  href="/pheniqs/2.0/install.html">Install</a></li>
        <li><a                  href="/pheniqs/2.0/build.html">Build</a></li>
        <li><a class="active"   href="/pheniqs/2.0/workflow.html">Workflows</a></li>
        <li><a                  href="/pheniqs/2.0/best_practices.html">Best Practice</a></li>
        <li><a                  href="/pheniqs/2.0/glossary.html">Glossary</a></li>
        <li><a                  href="/pheniqs/2.0/manual.html">Manual</a></li>
        <li><a                  href="/pheniqs/2.0/cli.html">CLI</a></li>
        <li><a class="github"   href="http://github.com/biosails/pheniqs">View on GitHub</a></li>
    </ul>
    <div class="clear" />
</section>

<a name="overview" />
General framework for sample multiplexing (Illumina platform)
: A large number of multiplexing designs have been devised for the many short-read sequencing applications currently available. While the specifics will vary, the basic information to be extracted is always similar:
- biological sequences of interest: inserts, usually of unknown composition (e.g. cDNA or genomic DNA)
- sample barcode tags: barcodes, usually 8-16nt in length, that discriminate between experimental samples (e.g. treatment / controls, biological replicates) 
- any additional barcode tags (e.g. cellular barcodes, unique molecular indexes (UMIs), etc.).

Three standard schemes are as follows:

<!--- insert diagram here --->

The different pieces of information to be extracted will be distributed across 3 (for single indexing) or 4 (for dual indexing) read segments. For a standard dual-indexed application with a UMI, the read segments would look like this: 

<!--- insert diagram here --->

Pheniqs takes each of these read segments and creates "tokens" out of them that specify how the biological sequences, sample barcodes, etc. are distributed across them. Each token specifies a range of coordinates within a particular read segment, given by an offset and length. For example, an 8 nucleotide barcode tag starting from the beginning of the Index 1 (i7) read segment would be specified as "0:0:8". 

Pheniqs provides complete flexibility in specifying where to look for different tokens: they can be defined in any location, and can be combined in any abritrary fashion to accommodate combinatorial indexing designs. 

The vignettes in this section walk through how to configure Pheniqs for a variety of design scenarios.


<a name="standard_illumina" />
Standard Illumina dual-index, paired-end sample demultiplexing
: This [vignette](standard_illumina.html) illustrates the Pheniqs configuration for demultiplexing a sequencing run with paired-end dual-index samples that are multiplexed using the standard Illumina i5 and i7 indexing protocol with the [PAMLD decoder](glossary.html#phred_adjusted_maximum_likelihood_decoding).

<a name="fluidigm" />
Classifying single index fluidigm with a single cellular tag
: This [vignette](fluidigm.html) will walk you through a single index fluidigm sequencing run with the [PAMLD decoder](glossary.html#phred_adjusted_maximum_likelihood_decoding).
