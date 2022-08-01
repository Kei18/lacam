# experimental scripts for the MAPF benchmark

import CSV
import Dates
import Base.Threads
import YAML
import Glob: glob
using DataFrames
using Query
import Statistics: mean, median

function print_summary(csv_filename::String)
    df = CSV.File(csv_filename) |> DataFrame
    num_total = df |> @count()
    num_solved = df |> @filter(_.solved == 1) |> @count()
    if num_solved == 0
        println("solved\t$(num_solved)/$(num_total)=$(num_solved/num_total)")
        return
    end
    comp_time = df |> @filter(_.solved == 1) |> @map(_.comp_time) |> collect
    soc = df |> @filter(_.solved == 1) |> @map(_.soc / _.soc_lb) |> collect
    makespan = df |> @filter(_.solved == 1) |> @map(_.makespan / _.makespan_lb) |> collect
    r = (x) -> round(x,digits=3)
    describe = (x) -> "max=$(r(maximum(x)))\tmean=$(r(mean(x)))\tmed=$(r(median(x)))"
    println(
        "solved\t$(num_solved)/$(num_total)=$(num_solved/num_total)" *
            "\ncomp_time(ms)\t$(describe(comp_time))" *
            "\nsum_of_costs/lb\t$(describe(soc))" *
            "\nmakespan/lb\t$(describe(makespan))"
    )
end

function main(config_file)
    # load experimental setting
    config = YAML.load_file(config_file)
    seeds = get(config, "seeds", 5)
    time_limit_sec = get(config, "time_limit_sec", 10)
    scen = get(config, "scen", "test")
    num_min_agents = get(config, "num_min_agents", 10)
    num_interval_agents = get(config, "num_interval_agents", 10)
    maps = get(config, "maps", Vector{String}())
    all_scen = glob("assets/scen/scen-$(scen)/*.scen")
    date_str = replace(string(Dates.now()), ":" => "-")
    root_dir = joinpath(@__DIR__, "..", "..", "data", "exp", date_str)
    !isdir(root_dir) && mkpath(root_dir)
    additional_info = Dict(
        "git_hash" => read(`git log -1 --pretty=format:"%H"`, String),
        "date" => date_str,
    )
    YAML.write_file(joinpath(root_dir, "config.yaml"), merge(config, additional_info))

    l = 0
    for scen_file in all_scen
        lines = readlines(scen_file)
        N_max = length(lines) - 1
        map_name_raw = last(split(match(r"\d+\t(.+).map\t(.+)", lines[2])[1], "/"))
        !(map_name_raw in maps) && continue
        l += 1
        map_file = "assets/map/mapf-map/$(map_name_raw).map"
        cnt_fin = Threads.Atomic{Int}(0)
        loops = collect(enumerate(Iterators.product(num_min_agents:num_interval_agents:N_max, 1:seeds)))
        num_total_tasks = length(loops)
        result = Vector{Any}(undef, num_total_tasks)
        scen_short = first(split(last(split(scen_file, "/")), "."))
        println("$(l)/$(length(maps)*25)\t$(scen_short)")

        # solve, with multi-threading
        Threads.@threads for (k, (N, seed)) in loops
            output_file = "build/result-$(k).txt"
            run(`build/main -m $map_file -i $scen_file -N $N -o $output_file -t $time_limit_sec -s $seed -l`)
            Threads.atomic_add!(cnt_fin, 1)
            print("\r$(cnt_fin[])/$(num_total_tasks) tasks have been finished")
            row = Dict(
                :solver => "Lacam",
                :num_agents => N,
                :map_name => last(split(map_file, "/")),
                :scen => scen_short,
                :scen_num => last(split(scen_short, "-")),
            )
            for line in readlines(output_file)
                m = match(r"soc=(\d+)", line)
                !isnothing(m) && (row[:soc] = parse(Int, m[1]))
                m = match(r"soc_lb=(\d+)", line)
                !isnothing(m) && (row[:soc_lb] = parse(Int, m[1]))
                m = match(r"makespan=(\d+)", line)
                !isnothing(m) && (row[:makespan] = parse(Int, m[1]))
                m = match(r"makespan_lb=(\d+)", line)
                !isnothing(m) && (row[:makespan_lb] = parse(Int, m[1]))
                m = match(r"comp_time=(\d+)", line)
                !isnothing(m) && (row[:comp_time] = parse(Int, m[1]))
                m = match(r"seed=(\d+)", line)
                !isnothing(m) && (row[:seed] = parse(Int, m[1]))
                m = match(r"solved=(\d+)", line)
                !isnothing(m) && (row[:solved] = parse(Int, m[1]))
            end
            result[k] = NamedTuple{Tuple(keys(row))}(values(row))
            rm(output_file)
        end

        # save result
        result_file = joinpath(root_dir, "result-$(scen_short).csv")
        CSV.write(result_file, result)
        println()
        print_summary(result_file)
        l != length(maps) && println()
    end
end
