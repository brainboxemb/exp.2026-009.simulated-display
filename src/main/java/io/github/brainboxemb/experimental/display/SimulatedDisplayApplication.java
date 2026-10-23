package io.github.brainboxemb.experimental.display;

import javafx.application.Application;

/**
 * Plain Java entry point used by NetBeans and Maven.
 *
 * <p>The launcher deliberately does not extend JavaFX {@link Application}.
 * Starting an Application subclass directly with a plain Java launcher can
 * trigger "JavaFX runtime components are missing" even when JavaFX is present
 * on Maven's classpath.</p>
 */
public final class SimulatedDisplayApplication {

    private SimulatedDisplayApplication() {
    }

    public static void main(String[] args) {
        Application.launch(SimulatedDisplayFxApplication.class, args);
    }
}
