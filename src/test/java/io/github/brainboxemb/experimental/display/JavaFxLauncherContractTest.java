package io.github.brainboxemb.experimental.display;

import static org.junit.jupiter.api.Assertions.assertTrue;

import java.lang.reflect.Modifier;
import javafx.application.Application;
import org.junit.jupiter.api.Test;

final class JavaFxLauncherContractTest {

    @Test
    void javafxApplicationHasPublicNoArgConstructor() throws Exception {
        Class<?> applicationClass = SimulatedDisplayFxApplication.class;

        assertTrue(Application.class.isAssignableFrom(applicationClass));
        assertTrue(Modifier.isPublic(applicationClass.getModifiers()));
        applicationClass.getConstructor();
    }
}
