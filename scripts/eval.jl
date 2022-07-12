import CSV
import Dates
import Base.Threads
import Glob: glob
include("summary.jl")

function main(label = "test")
    seeds = 5
    time_limit_sec = 10

    date_str = replace(string(Dates.now()), ":" => "-")
    root_dir = joinpath(@__DIR__, "..", "..", "data", "exp", date_str)
    !isdir(root_dir) && mkpath(root_dir)
    all_scen = glob("assets/scen/scen-$(label)/*.scen")
    all_scen_num = length(all_scen)

    for (l, scen_file) in enumerate(all_scen)
        lines = readlines(scen_file)
        N_max = length(lines) - 1
        map_file = joinpath(
            "assets/map/mapf-map",
            match(r"\d+\t(.+).map\t(.+)", lines[2])[1]*".map"
        )
        cnt_fin = Threads.Atomic{Int}(0)
        loops = collect(enumerate(Iterators.product(2:N_max, 1:seeds)))
        num_total_tasks = (N_max - 1) * seeds
        result = Vector{Any}(undef, num_total_tasks)
        scen_short = first(split(last(split(scen_file, "/")), "."))
        println("$(l)/$(all_scen_num)\t$(scen_short)")

        # solve
        Threads.@threads for (k, (N, seed)) in loops
            output_file = "build/result-$(k).txt"
            run(`build/main -m $map_file -i $scen_file -N $N -o $output_file -t $time_limit_sec -s $seed -l`)
            Threads.atomic_add!(cnt_fin, 1)
            print("\r$(cnt_fin[])/$(num_total_tasks) tasks have been finished")
            row = Dict(:N => N, :map_name => last(split(map_file, "/")), :scen => scen_short)
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
        l != all_scen_num && println()
    end
end
