<home data-page="true">

    <header class="header-bar">
        <div class="center">
            <h1 class="title">Curtain Control</h1>
        </div>
    </header>          
    <div class="content">
      <ul class="list">
        <li class="divider">Steuerung</li>
        <li class="action">
        <statuspanel></statuspanel>
        </li>
        <li class="action" data-action="open">
            <i class="pull-left icon icon-arrow-back"></i>
            <span class="padded-list">Öffnen</span>
        </li>
        <li class="action" data-action="close">
            <i class="pull-left icon icon-arrow-forward"></i>
            <span class="padded-list">Schließen</span>
        </li>
        <li class="action" data-action="stop">
            <i class="pull-left icon icon-close"></i>
            <span class="padded-list">Stop</span>
        </li>
        <li class="action" data-action="calibrate">
            <i class="pull-left icon icon-sync"></i>
            <span class="padded-list">Kalibrieren</span>
        </li>
      </ul>
    </div>
    <style scoped>
      .action{cursor: pointer;}
    </style>
</home>