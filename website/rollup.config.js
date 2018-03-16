import jsx from 'rollup-plugin-jsx';
import commonjs from 'rollup-plugin-commonjs';
import nodeResolve from 'rollup-plugin-node-resolve';
import replace from 'rollup-plugin-replace';
import uglify from 'rollup-plugin-uglify';

export default {
  input: 'playground/try.js',
  name: 'tryReason',
  output: {
    file: 'static/js/try.js',
    format: 'iife'
  },
  plugins: [
    replace({
      'process.env.NODE_ENV': JSON.stringify( 'production' )
    }),
    jsx({
      factory: 'React.createElement',
      passUnknownTagsToFactory: true,
      arrayChildren: false,
    }),
    nodeResolve({
      jail: '/..',
    }),
    commonjs({
      include: [
        'node_modules/**',
      ],
      namedExports: {
        'node_modules/react/react.js': ['Children', 'Component', 'PropTypes', 'createElement'],
        'node_modules/react-dom/index.js': ['render'],
        'node_modules/lz-string/libs/lz-string.js': ['compressToEncodedURIComponent', 'decompressFromEncodedURIComponent'],
      }
    }),
    uglify(),
  ],
};
