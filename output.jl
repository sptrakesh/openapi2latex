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

function latex(o::OpenAPI, author::String, f::IOStream)
    write(f, """\\input{$(pwd())/structure.tex}

\\lhead{\\textsf{\\textbf{OpenAPI2\\LaTeX}}}
\\rhead{\\textsf{\\textbf{OpenAPI $(o.openapi)}}}
\\lfoot{\\textsf{\\textbf{Version $(o.info.version)}}}
\\rfoot{\\textsf{\\textbf{Proprietary and Confidential}}}

\\title{$(o.info.title)\\\\
Version: $(o.info.version)}
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
""")
end

function latex!(o::Operation, path::String, oplabels::OrderedDict{String,String}, f::IOStream)
    if !isempty(o.operationId)
        p = replace(path, "{" => "\\{")
        p = replace(p, "}" => "\\}")
        if haskey(oplabels, o.operationId)
            write(f, "\n\\section*{$(o.operationId)}\n")
            write(f, "\\seqsplit{$p}\n")
            write(f, "See Section \\ref{$(oplabels[o.operationId])} on page \\pageref{$(oplabels[o.operationId])}\n")
        else
            id = "opertion:$(o.operationId)"
            write(f, "\n\\section{\\label{$id}$(o.operationId)}\n")
            write(f, "\\seqsplit{$p}\n")
            if !isempty(o.summary) write(f, "\\begin{quote}$(convert(o.summary))\\end{quote}\n") end
            if !isempty(o.description) write(f, "$(convert(o.description))\n") end
            if o.externalDocs isa ExternalDocumentation
                if !isempty(o.externalDocs.description) write(f, "$(convert(o.externalDocs.description))\n") end
                uri = uristring(o.externalDocs.url)
                if !isempty(uri) write(f, "\\href{$uri}{$uri}\n") end
            end

            if !isempty(o.parameters)
                write(f, "\\subsection{\\label{$id:parameters}Parameters}\n")
                write(f, "\\begin{itemize}\n")
                for p in o.parameters
                    write(f, "\\item \\textbf{$(p.name)} ")
                    if !isempty(p.description) write(f, convert(p.description)) end
                    write(f, "\\begin{description}\n")
                    write(f, "\\item \\textit{in} - $(p.in)\n")
                    write(f, "\\item \\textit{required} - $(p.required)\n")
                    if !isempty(p.schema.type)
                        if !isempty(p.schema.description)
                            write(f, "\\item \\textit{schema} - $(convert(p.schema.description))\n")
                        else
                            write(f, "\\item \\textit{schema}\n")
                        end
                        write(f, "\\begin{description}\n")
                        write(f, "\\item \\textit{type} - $(p.schema.type)\n")
                        if !isempty(p.schema.enum)
                            write(f, "\\item \\textit{enum} - Allowed values \\texttt{")
                            write(f, join(p.schema.enum, ", "))
                            write(f, "}\n")
                        end
                        if p.schema.maximum isa Number write(f, "\\item \\textit{maximum} - $(p.schema.maximum)\n") end
                        if p.schema.minimum isa Number write(f, "\\item \\textit{minimum} - $(p.schema.minimum)\n") end
                        if !isempty(p.schema.pattern) write(f, "\\item \\textit{pattern} - \\texttt{$(p.schema.pattern)}\n") end
                        if !isempty(p.schema.format) write(f, "\\item \\textit{format} - $(p.schema.format)\n") end
                        if !isempty(p.schema.example) write(f, "\\item \\textit{example} - \\texttt{$(p.schema.example)}\n") end
                        if !isempty(p.schema.default) write(f, "\\item \\textit{default} - \\texttt{$(p.schema.default)}\n") end
                        write(f, "\\end{description}\n")
                    end
                    write(f, "\\end{description}\n")
                end
                write(f, "\\end{itemize}\n")
            end

            if o.requestBody isa RequestBody
                write(f, "\\subsection{\\label{$id:requestBody}Request Body}\n")
                write(f, "\\begin{quote}\\textbf{Required} - \\texttt{$(o.requestBody.required)}\\end{quote}")
                if !isempty(o.requestBody.description) write(f, "$(convert(o.requestBody.description))\n") end
                ref = "schema:$(entity(o.requestBody.ref))"
                if !isempty(o.requestBody.ref) write(f, "See Chapter \\ref{$ref} on \\pageref{$ref}") end

                if !isempty(o.requestBody.content)
                    write(f, "\\begin{description}\n")
                    for (c,m) in o.requestBody.content
                        write(f, "\\item \\textbf{$c}\n")
                        if m.schema isa Schema
                            if !isempty(m.schema.summary) write(f, "$(convert(m.schema.summary))\n") end
                            if !isempty(m.schema.description) write(f, "$(convert(m.schema.description))\n") end
                            if !isempty(m.schema.ref)
                                ref = "schema:$(entity(m.schema.ref))"
                                write(f, "See Chapter \\ref{$ref} on \\pageref{$ref}")
                            end
                        end
                    end
                    write(f, "\\end{description}\n")
                end
            end

            if !isempty(o.responses)
                write(f, """\\subsection{\\label{$id:responses}Responses}
See table \\ref{$id:responses:table} for response codes and data.
""")
                if isempty(o.responses)
                    write(f, "No data returned\n")
                else
                    write(f, """\\begin{table}[h!]
    \\centering
    \\begin{supertabular}{|l|l|p{80mm}|}
    \\hline Code & Type & Details \\\\
    """)
                    for (code,resp) in o.responses
                        for (ct,sc) in resp.content
                            if !(sc.schema isa Schema) continue end
                            if !isempty(sc.schema.ref)
                                desc = convert(resp.description)
                                if !isempty(desc) desc = desc * "\n\n" end
                                ref = "schema:$(entity(sc.schema.ref))"
                                write(f, "\\hline $code & $ct & $desc See Chapter \\ref{$ref} on \\pageref{$ref} for schema. \\\\\n")
                            else
                                write(f, "\\hline $code & $ct & ")
                                if !isempty(resp.description) write(f, "$(convert(resp.description))\n\n") end
                                if !isempty(sc.schema.summary) write(f, "$(convert(sc.schema.summary))\n\n") end
                                if !isempty(sc.schema.description) write(f, "$(convert(sc.schema.description))\n\n") end

                                if !isempty(sc.schema.properties)
                                    write(f, "\\begin{description}\n")
                                    for (p, prop) in sc.schema.properties write(f, "\\item \\textbf{p}\n") end
                                    write(f, "\\end{description}\n")
                                end
                                write(f, "\\\\\n")
                            end
                        end
                    end

            write(f, """
    \\hline
    \\end{supertabular}
    \\caption{Responses for $(o.operationId)}
    \\label{$id:responses:table}
    \\end{table}
    """)
                end
            end

            oplabels[o.operationId] = id
        end
    end
end

import JSON: json
function latex(schema::Schema, key::String, f::IOStream)
    @debug "Writing output for schema $key"
    id = "schema:$key"
    write(f, "\n\\chapter{\\label{$id}$(isempty(schema.title) ? key : schema.title)}\n")
    if !isempty(schema.summary) write(f, "\\begin{quote}$(convert(schema.summary))\\end{quote}\n") end
    if !isempty(schema.description) write(f, "$(convert(schema.description))\n") end
    if isempty(schema.properties) return end

    function required(name::String)
        for r in schema.required
            if r == name return true end
        end
        false
    end

    function clean(s::String)::String
        ret = replace(s, "&" => "\\&")
        ret = replace(ret, "\$" => "\\\$")
        ret = replace(s, "[" => "\\[")
        ret = replace(s, "]" => "\\]")
        ret = replace(s, "{" => "\\{")
        ret = replace(s, "}" => "\\}")
        replace(ret, "#" => "\\#")
    end

    function refkey(k::String)::String
        res = entity(k)
        if !startswith(res, "::") return res end
        first(split(key, "::")) * res
    end

    for (name, prop) in schema.properties
        pid = "$id:$name"
        write(f, "\\section{\\label{$pid}$name}\n")
        if !isempty(prop.summary) write(f, "\\begin{quote}$(convert(prop.summary))\\end{quote}\n") end
        if !isempty(prop.description) write(f, "$(convert(prop.description))\n") end

        example = prop.example isa String ? prop.example : ""
        def = prop.default isa String ? prop.default : ""

        write(f, """\\begin{table}[h!]
\\centering
\\begin{supertabular}{|l|l|}
\\hline Type & $(prop.type) \\\\
\\hline Required & $(required(name)) \\\\
""")

        if !isempty(prop.ref) write(f, "\\hline Reference & See section \\ref{schema:$(refkey(prop.ref))} on page \\pageref{schema:$(refkey(prop.ref))}. \\\\\n") end
        if !isempty(def) write(f, "\\hline Default & $(clean(def)) \\\\\n") end
        if !isempty(prop.pattern) write(f, "\\hline Pattern & \\verb $(prop.pattern) \\\\\n") end
        if !isempty(prop.format) write(f, "\\hline Format & $(clean(prop.format)) \\\\\n") end
        if !isempty(example) write(f, "\\hline Example & $(clean(example)) \\\\\n") end
        if prop.maximum isa Number write(f, "\\hline Maximum & $(prop.maximum) \\\\\n") end
        if prop.exclusiveMaximum isa Number write(f, "\\hline Exclusive Maximum & $(prop.exclusiveMaximum) \\\\\n") end
        if prop.minimum isa Number write(f, "\\hline Minimum & $(prop.minimum) \\\\\n") end
        if prop.exclusiveMinimum isa Number write(f, "\\hline Exclusive Minimum & $(prop.exclusiveMinimum) \\\\\n") end
        if prop.maxLength isa Number write(f, "\\hline Max Length & $(prop.maxLength) \\\\\n") end
        if prop.minLength isa Number write(f, "\\hline Min Length & $(prop.minLength) \\\\\n") end
        if prop.maxItems isa Number write(f, "\\hline Max Items & $(prop.maxItems) \\\\\n") end
        if prop.minItems isa Number write(f, "\\hline Min Items & $(prop.minItems) \\\\\n") end

        if !isempty(prop.enum)
            e = join(prop.enum, ",")
            write(f, "\\hline Enum & Allowed values - \\texttt{$e} \\\\\n")
        end

        if prop.nullable isa Bool write(f, "\\hline Nullable & $(prop.nullable) \\\\\n") end
        if prop.readOnly isa Bool write(f, "\\hline Read Only & $(prop.readOnly) \\\\\n") end
        if prop.writeOnly isa Bool write(f, "\\hline Write Only & $(prop.writeOnly) \\\\\n") end
        if prop.deprecated isa Bool write(f, "\\hline Deprecated & $(prop.deprecated) \\\\\n") end

        write(f, """
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
    (pschemas, responses) = collect_paths!(o, args["input"])
    schemas = collect_schemas!(o, args["input"], pschemas)
    tags = collect_tag_paths(o)

    open(args["output"], "w") do f
        latex(o, args["author"], f)
        oplabels = OrderedDict{String,String}()

        for tag in o.tags
            @debug "Adding operations for tag $(tag.name)"
            write(f, "\n\\chapter{$(tag.name)}\n")
            if !isempty(tag.description) write(f, "\\begin{quote}$(tag.description)\\end{quote}\n") end
            if !haskey(tags, tag.name) @warn "No operations found for Tag with name $(tag.name)!"; continue end
            for (p,pi) in tags[tag.name]
                latex!(pi.get, p, oplabels, f)
                latex!(pi.put, p, oplabels, f)
                latex!(pi.post, p, oplabels, f)
                latex!(pi.delete, p, oplabels, f)
                latex!(pi.options, p, oplabels, f)
                latex!(pi.head, p, oplabels, f)
                latex!(pi.patch, p, oplabels, f)
                latex!(pi.trace, p, oplabels, f)
            end
        end

        write(f, "\\part{Schemas}")
        for (key,schema) in schemas latex(schema, key, f) end

        write(f, """\\backmatter
\\printindex % Print the index at the very end of the document
\\end{document}""")
    end
end
