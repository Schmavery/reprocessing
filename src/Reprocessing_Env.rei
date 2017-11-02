let width: Reprocessing_Types.Types.glEnvT => int;

let height: Reprocessing_Types.Types.glEnvT => int;

let mouse: Reprocessing_Types.Types.glEnvT => (int, int);

let pmouse: Reprocessing_Types.Types.glEnvT => (int, int);

let mousePressed: Reprocessing_Types.Types.glEnvT => bool;

let keyCode: Reprocessing_Types.Types.glEnvT => Reprocessing_Events.keycodeT;

let size: (~width: int, ~height: int, Reprocessing_Types.Types.glEnvT) => unit;

let resizeable: (bool, Reprocessing_Types.Types.glEnvT) => unit;

let frameRate: Reprocessing_Types.Types.glEnvT => int;

let frameCount: Reprocessing_Types.Types.glEnvT => int;


/*** Time in seconds since the last frame */
let deltaTime: Reprocessing_Types.Types.glEnvT => float;
