<template>
    <div class="content">
        <header>
			<h1>SoupedModFramework</h1>
		</header>
		<div class="body">
            <Settings/>
		</div>
		<footer>
			<LaunchButton />
		</footer>
	</div>
</template>

<script>
import LaunchButton from '../components/LaunchButton.vue'
import Settings from '../components/Settings.vue'

export default {
	components: {
		LaunchButton,
        Settings
	},
	data() {
		return {
		}
	},
	methods: {  
        onResize() {
        },
        isOnLauncher() {
            try {
                return areWeNative();
            } catch {
                console.log("We are not on native!");
                return false;
            }
        }
    }
}
</script>

<style lang="stylus">
@import url('https://fonts.googleapis.com/css2?family=Roboto:wght@100&display=swap');

html, body {
	margin:0px;
	height:100%;
	background-color: #131313;
	color: #FFFFFF;
	font-family: 'Roboto', sans-serif;
}

.browserWarn {
    border-style: solid;
    border-color: #FF0000;
    background-color: #FF00007F;
	grid-row: 2 / 8;
    grid-column: 3;
    overflow-y: scroll;
}

.content {
	position: absolute;
	width: 100vw;
	height: 100vh;
	background-color: #131313;
	color: #FFFFFF;
	display: grid;
	grid-template-rows: repeat(10, 1fr);
	grid-template-columns: repeat(3, 1fr);
}

header {
	grid-column: 2;
}

.body {
	grid-row: 2 / 8;
    grid-column: 2;
}

footer {
	grid-row: 9;
	grid-column: 2;
	display: grid;
}

h1 {
	text-align: center;
}
</style>