export const numberToColour = (colourNum: number) => {
    if (colourNum == 0) return "white";
    else if (colourNum == 1) return "black";
    else return "INVALID";
}