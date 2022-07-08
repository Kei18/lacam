import CSV
using DataFrames
using Query
using Plots
import Statistics: mean, median

function safe_savefig!(filename::Union{Nothing,String} = nothing)
    isnothing(filename) && return
    dirname = join(split(filename, "/")[1:end-1], "/")
    !isdir(dirname) && mkpath(dirname)
    savefig(filename)
end

function print_summary(csv_filename::String)
    df = CSV.File(csv_filename) |> DataFrame
    num_total = df |> @count()
    num_solved = df |> @filter(_.solved == 1) |> @count()
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

function plot_cactus(
    csv_filename::String;
    result_filename::String = (first(csv_filename) == '/' ? "/" : "") *
                              joinpath(split(csv_filename, "/")[1:end-1]..., "cactus_plot.pdf")
)
    df = CSV.File(csv_filename) |> DataFrame
    plot(
        xlims = (0, df |> @count()),
        ylims = (1, df |> @map(_.comp_time) |> collect |> maximum),
        xlabel = "solved instances",
        ylabel = "runtime (ms)",
        yaxis = :log,
    )
    Y = df |> @filter(_.solved == 1) |> @map(_.comp_time) |> collect |> sort
    X = collect(1:length(Y))
    plot!(X, Y, linetype = :steppost, linewidth = 3, label = nothing)
    safe_savefig!(result_filename)
end
