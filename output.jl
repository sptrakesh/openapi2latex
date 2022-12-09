function table(i::Info)::String
    if isempty(i.termsOfService.scheme) && isempty(i.version) && isempty(i.license.name) return "" end
    s = """\\section{Other Information}
\\begin{table}[ht]
\\centering
\\begin{tabular}{|l|l|}
"""
    if !isempty(i.termsOfService.scheme) s = s * "\\hline Terms Of Service & \\href{$(uristring(i.termsOfService))}{$(uristring(i.termsOfService))} \\\\\n" end
    if !isempty(i.version) s = s * "\\hline Version & $(i.version) \\\\\n" end
    if !isempty(i.license.name) && !isempty(i.license.identifier) s = s * "\\hline License & $(i.license.identifier) \\\\\n"
    elseif !isempty(i.license.name) && !isempty(i.license.url.scheme) s = s * "\\hline License & \\href{$(i.license.name)}{$(uristring(i.license.url))} \\\\\n"
    end
    s * """\\hline
\\end{tabular}
\\caption{API Information}
\\end{table}
"""
end

function servers(o::OpenAPI)::String
    if isempty(o.servers) return "" end

    t = """\\begin{table}[ht]
\\centering
\\begin{tabular}{|l|l|}
"""

    for s in o.servers
        t = t * "\\hline $(s.description) & \\href{$(uristring(s.url))}{$(uristring(s.url))} \\\\\n"
    end

    t * """\\hline
\\end{tabular}
\\caption{Server Information}
\\end{table}
"""
end


function security_schemes(c::Components)::String
    if isempty(c.securitySchemes) return "" end

    t = """\\begin{table}[ht]
\\centering
\\begin{tabular}{|l|l|l|}
"""
    for (key, s) in c.securitySchemes
        count = 0
        if !isempty(s.type) count += 1 end
        if !isempty(s.description) count += 1 end
        if !isempty(s.name) count += 1 end
        if !isempty(s.in) count += 1 end
        if !isempty(s.scheme) count += 1 end
        if !isempty(s.bearerFormat) count += 1 end
        if !isempty(s.openIdConnectUrl.scheme) count += 1 end
        t = t * "\\hline \\multirow{$count}{*}{\\textbf{$key}} & Type & $(s.type) \\\\\n"
        if !isempty(s.description) t = t * "\\cline{2-3} & Description & $(s.description) \\\\\n" end
        if !isempty(s.name) t = t * "\\cline{2-3} & Name & $(s.name) \\\\\n" end
        if !isempty(s.in) t = t * "\\cline{2-3} & In & $(s.in) \\\\\n" end
        if !isempty(s.scheme) t = t * "\\cline{2-3} & Scheme & $(s.scheme) \\\\\n" end
        if !isempty(s.bearerFormat) t = t * "\\cline{2-3} & Bearer Format & $(s.bearerFormat) \\\\\n" end
        if !isempty(s.openIdConnectUrl.scheme) t = t * "\\cline{2-3} & OpenId Connect & \\href{$(uristring(s.openIdConnectUrl))}{$(uristring(s.openIdConnectUrl))} \\\\\n" end
    end

    t * """\\hline
\\end{tabular}
\\caption{Security Schemes}
\\end{table}
"""
end

function latex!(o::Operation, oplabels::OrderedDict{String,String}, f::IOStream)
    if !isempty(o.operationId)
        if haskey(oplabels, o.operationId)
            write(f, "\n\\section*{$(o.operationId)}\n")
            write(f, "See Section \\ref{$(oplabels[o.operationId])} on page \\pageref{$(oplabels[o.operationId])}")
        else
            id = "opertion:$(o.operationId)"
            write(f, "\n\\section{\\label{$id}$(o.operationId)}\n")
            if !isempty(o.summary) write(f, "\\begin{quote}$(convert(o.summary))\\end{quote}\n") end
            if !isempty(o.description) write(f, "$(convert(o.description))\n") end
            if !isempty(o.parameters)
                write(f, "\\subsection{\\label{$id:parameters}Parameters}\n")
                write(f, "\\begin{description}\n")
                for p in o.parameters
                    write(f, "\\item \\textbf{$(p.name)} ")
                    if !isempty(p.description) write(f, convert(p.description)) end
                    write(f, "\\begin{description}\n")
                    write(f, "\\item \\textit{in} - $(p.in)\n")
                    write(f, "\\item \\textit{required} - $(p.required)\n")
                    write(f, "\\end{description}\n")
                end
                write(f, "\\end{description}\n")
            end
            oplabels[o.operationId] = id
        end
    end
end

import JSON: json
function latex(schema::Schema, key::String, f::IOStream)
    @debug "Writing output for schema $key"
    id = "schema:$key"
    write(f, "\n\\chapter{\\label{$id}$key}\n")
    if !isempty(schema.summary) write(f, "\\begin{quote}$(convert(schema.summary))\\end{quote}\n") end
    if !isempty(schema.description) write(f, "$(convert(schema.description))\n") end
    if isempty(schema.properties) return end

    function required(name::String)
        for r in schema.required
            if r == name return true end
        end
        false
    end

    for (name, prop) in schema.properties
        pid = "$id:$name"
        write(f, "\\section{\\label{$pid}$name}\n")
        if !isempty(prop.summary) write(f, "\\begin{quote}$(convert(prop.summary))\\end{quote}\n") end
        if !isempty(prop.description) write(f, "$(convert(prop.description))\n") end
        example = prop.example isa String ? prop.example : ""
        example = replace(example, "&" => "\\&")
        example = replace(example, "\$" => "\\\$")
        example = replace(example, "#" => "\\#")

        write(f, """\\begin{table}[h!]
\\centering
\\begin{supertabular}{|l|p{100mm}|}
\\hline Type & $(prop.type) \\\\
\\hline Required & $(required(name)) \\\\
\\hline Default & $(prop.default) \\\\
\\hline Example & $(example) \\\\
\\hline
\\end{supertabular}
\\caption{Properties for $key::$name}
\\end{table}
""")

        if prop.example isa OrderedDict{Any,Any}
            write(f, """\\subsubsection*{Code Example}
\\begin{lstlisting}
$(json(prop.example, 2))
\\end{lstlisting}""")
        end
    end
end

function generate!(o::OpenAPI, args::Dict{String,Any})
    collect_tags!(o)
    collect_paths!(o, args["input"])
    schemas = collect_schemas!(o, args["input"])
    tags = collect_tag_paths(o)

    author = args["author"]
    head = """\\input{$(pwd())/structure.tex}

\\lhead{\\textsf{\\textbf{OpenAPI2\\LaTeX}}}
\\rhead{\\textsf{\\textbf{OpenAPI $(o.openapi)}}}
\\lfoot{\\textsf{\\textbf{Version $(o.info.version)}}}
\\rfoot{\\textsf{\\textbf{Proprietary and Confidential}}}

\\title{$(o.info.title)\\\\
$(o.info.version)}
\\author{$(author)}
\\date{\\today}

\\begin{document}

\\frontmatter
% Title Page
\\thispagestyle{empty}
\\maketitle

%\\newpage
\\tableofcontents

\\listoftables

\\clearpage
\\mainmatter
\\chapter{Information}
\\begin{quote}$(convert(o.info.summary))\\end{quote}

$(o.info.description)

$(table(o.info))

$(servers(o))

$(security_schemes(o.components))

\\part{Endpoints}
"""

    open(args["output"], "w") do f
        write(f, head)
        oplabels = OrderedDict{String,String}()


        for tag in o.tags
            @debug "Adding operations for tag $(tag.name)"
            write(f, "\n\\chapter{$(tag.name)}\n")
            if !isempty(tag.description) write(f, "\\begin{quote}$(tag.description)\\end{quote}\n") end
            if !haskey(tags, tag.name) @warn "No operations found for Tag with name $(tag.name)!"; continue end
            for pi in tags[tag.name]
                latex!(pi.get, oplabels, f)
                latex!(pi.put, oplabels, f)
                latex!(pi.post, oplabels, f)
                latex!(pi.delete, oplabels, f)
                latex!(pi.options, oplabels, f)
                latex!(pi.head, oplabels, f)
                latex!(pi.patch, oplabels, f)
                latex!(pi.trace, oplabels, f)
            end
        end

        write(f, "\\part{Schemas}")
        for (key,schema) in schemas latex(schema, key, f) end

        write(f, """\\backmatter
\\bibliography{bibliography} % Use the bibliography.bib file for the bibliography
\\bibliographystyle{plainnat} % Use the plainnat style of referencing
\\printindex % Print the index at the very end of the document
\\end{document}""")
    end
end
