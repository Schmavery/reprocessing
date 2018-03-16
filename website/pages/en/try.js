const React = require("react");

class Index extends React.Component {
  render() {
    return <span>
      <div id="try-wrapper" />
      <script type="text/javascript" src="/reprocessing/js/bs.js" />
      <script type="text/javascript" src="/reprocessing/js/refmt.js" />
      <script type="text/javascript" src="/reprocessing/js/try.js" />
    </span>;
  }
}

module.exports = Index;
