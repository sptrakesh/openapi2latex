# OpenAPI2LaTeX
* [Structure](#structure)
* [Usage](#usage)
  * [Install Dependencies](#install-dependencies)
  * [Command Line Options](#command-line-options)
* [Extension](#extensions)
* [Limitations](#limitations)

Utility scripts to generate a LaTeX file from an OpenAPI specification.

The rationale for this script is to be able to generate a PDF document that can be distributed to interested
parties, when the source specifications are protected by access control (and where the said parties do not need to
be provided with the access credentials).

The workflow is to use these scripts to generate the target LaTeX file, and run `pdflatex` a few times (usually
two times to get cross-references resolved) for the output PDF document.

## Structure
The generated LaTeX file has the following structure (you can of course modify the output file as desired):

* **Frontmatter** - Titlepage and table of contents.
* **Mainmatter** - Contains two or three parts.
  * **Info** - The *info* object is presented as the first chapter. 
  * **Endpoints** - Part with the path operations grouped by tags. Each tag is presented in a *chapter*.
  * **Schemas** - Part with the *schemas* declared and referenced in the specification.  Each *schema* is presented in a
    *chapter*.  Schemas are listed alphabetically by their filename and entity name.
  * **Code Samples** - If the `x-codeSamples` extension exists for operations, these are collected together into a third
    *part*.  Code samples are grouped together under each *tag* group, which is presented as a *chapter*.
* **Backmatter** - List of tables.

## Usage
The generator is written in [Julia](https://julialang.org/), and hence requires it to be installed on the target computer.

The set of scripts are not packaged as a Julia Package, since it is not designed for re-use in other systems.  The
[Project.toml](Project.toml) file lists all the dependencies as regular packages would.

```shell
cd <path to checked out scripts>
<path to>/julia main.jl -i <path to>/openapi.yaml -o /tmp/openapi.tex -d
cd /tmp
pdflatex openapi.tex
pdflatex openapi.tex
pdflatex openapi.tex # if output says run again
open openapi.pdf
```

### Install Dependencies

Start the Julia Pkg REPL.

```julia
julia> ]
pkg> add ArgParse
pkg> add OrderedCollections
pkg> add JSON
pkg> add MiniLoggers
pkg> add URIs
pkg> add YAML
pkg> [delete]
julia> CTRL+d
```

### Command Line Options
The following options are supported by the [main.jl](main.jl) script:

* `--input | -i` - **Required**. The main OpenAPI specification file to parse.
* `--output | -o` - **Required**. The output LaTeX file to generate.  Best to place this at another location than the api specs.
* `--author | -a` - The author credit to show on the titlepage.
* `--debug | -d` - Show debug log messages.

## Extensions
A few extensions to the specifications developed by [Redocly](https://redocly.com/) are supported. 

* Source code samples are parsed from the `x-codeSamples` array attached to an operation.  All code samples are attached
  to a separate *part* of the output document, and follow the same chapter organisation as the API tags.
* Tag groups are parsed from the `x-tagGroups` array attached to the root of the specification document.  If specified
  an initial chapter **Tag Groups** is added, which lists the groupings with links to the **Tag** chapters.

## Limitations
Probably too many to list, but the following items should be kept in mind.

* These scripts are based on the way *I write API specifications*, and markup descriptions.
* Schema objects are assumed to model closely their organisation in a source code implementation.  This in turn implies
  that nested structures are represented as schema references, and not listed in-line in the schema.  Deeply nested in-line
  schemas would be very hard to represent in a printed document in any case.
* Mainly tested with specifications that are split into individual files - representing paths, schemas, parameters etc.
  Most testing has been against large handwritten specifications, which follows the principles laid
  out in [split specifications](https://davidgarcia.dev/posts/how-to-split-open-api-spec-into-multiple-files/).  A few
  simple single file specifications have also been tested.
* Mainly supports OpenAPI specification version [3.0.3](https://spec.openapis.org/oas/v3.0.3), although some properties
  from [3.1.0](https://spec.openapis.org/oas/latest.html) are also included.  In particular, schemas are expected in
  3.0.3 format.
* Only supports loading local specification files in YAML format.  JSON is not supported at present.
* Not all properties/aspects of the specification are output in the generated LaTeX file.  I selected what I felt are
  most relevant to be shared.
* Markdown markup may not be fully translated to LaTeX.  See [runtests.jl](runtests.jl) for basic rules implemented.
  * Bold/italic blocks of text (spanning paragraphs) are not supported.

The output is a LaTeX file, and hence can be easily modified as needed to further customise the final PDF document.